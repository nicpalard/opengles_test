#include <opencv2/opencv.hpp>
#include <hiredis/hiredis.h>

#include "quad.hpp"
#include "gles_utils.hpp"
#include <RedisCameraClient.hpp>
#include "RedisCameraServer.hpp"
#include "ImageUtils.hpp"

static bool isPowerOfTwo(int x)
{
    return (x != 0) && ((x & (x - 1)) == 0);
}

int main(int argc, char** argv)
{
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <vertex shader path> <fragment shader path> <output file> <fake|camera frame> <size>" << std::endl;
        return EXIT_FAILURE;
    }

    RedisCameraClient client;
    if (!client.connect()) { std::cerr << "Error: Could not connect to the server." << std::endl; return EXIT_FAILURE; }

    std::string cameraKey;
    //0. Prepare image texture
    if (strcmp(argv[4], "camera") == 0)
    {
        //Get image from webcam into redis
        RedisCameraServer server;
        std::string gstCommand = "nvcamerasrc ! video/x-raw(memory:NVMM), width=(int)1280, height=(int)720, format=(string)I420, framerate=(fraction)120/1, queue-size=2, blockSize=16384, auto-exposure=1, scene-mode=1, flicker=0"
                                "! nvvidconv flip-method=0 ! video/x-raw, format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink";

        if (!server.start(gstCommand)) { std::cerr << "Could not get webcam frame." << std::endl; return EXIT_FAILURE; }

        cameraKey = "custom:image";
        server.setCameraKey(cameraKey);
        server.pickUpCameraFrame();

        client.setCameraKey(cameraKey);
    }
    else
    {
        if (argc != 6)
        {
            std::cerr << "When using a fake frame, you should provide fake frame size as well." << std::endl;
            return EXIT_FAILURE;
        }
        int size = atoi(argv[5]);
        if (!isPowerOfTwo(size)) { std::cerr << "Error: Specified size must be power of 2." << std::endl; return EXIT_FAILURE; }

        unsigned char* tmpdata = float_to_uchar(generate_random_image(size, size, 3), size * size * 3);

        CameraFrame* frame = new CameraFrame(size, size, 3, tmpdata);
        cameraKey = "custom:image:fake";
        client.setCameraKey(cameraKey);
        client.setCameraFrame(frame);
    }

    // Get camera frame from redis
    CameraFrame* frame = client.getCameraFrame();
    if (frame == NULL)
    {
        return EXIT_FAILURE;
    }
    int image_width = frame->width(), image_height = frame->height();
    unsigned char* image = frame->data();

    //1. Get a EGL valid display
    EGLDisplay display;
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        std::cerr << "Failed to get EGL Display" << std::endl
            << "Error: " << eglGetError() << std::endl;
        return EXIT_FAILURE;
    }
    else {
        std::cerr << "Successfully get EGL Display." << std::endl;
    }


    //2. Initialize EGL. Create a connection to the display.
    int minor, major;
    if (eglInitialize(display, &minor, &major) == EGL_FALSE) {
        std::cerr << "Failed to initialize EGL Display" << std::endl
            << "Error: " << eglGetError() << std::endl;
        eglTerminate(display);
        return EXIT_FAILURE;
    }
    else {
        std::cerr << "Successfully intialized display (OpenGL ES version " << minor << "." << major << ")." << std::endl;
    }

    //3. Find a config that match specified requirements (in gles_utils.hpp).
    // OpenGL ES Config are used to specify things like multi sampling, channel size, stencil buffer usage, & more
    // See the doc: https://www.khronos.org/registry/EGL/sdk/docs/man/html/eglChooseConfig.xhtml for more informations
    EGLConfig config;
    EGLint num_configs;
    if (!eglChooseConfig(display, EGL_CONFIG_ATTRIBUTES, &config, 1, &num_configs)) {
        std::cerr << "Failed to choose EGL Config" << std::endl
            << "Error: " << eglGetError() << std::endl;
        eglTerminate(display);
        return EXIT_FAILURE;
    }
    else {
        std::cerr << "Successfully choose OpenGL ES Config ("<< num_configs << ")." << std::endl;
    }

    //4. Creating an OpenGL Render Surface with surface attributes defined above to draw to.
    EGLint pbufferAttributes[] = {EGL_WIDTH, image_width, EGL_HEIGHT, image_height, EGL_NONE};
    EGLSurface surface = eglCreatePbufferSurface(display, config, pbufferAttributes);
    if (surface == EGL_NO_SURFACE) {
        std::cerr << "Failed to create EGL Surface." << std::endl
            << "Error: " << eglGetError() << std::endl;
    }
    else {
        std::cerr << "Successfully created OpenGL ES Surface." << std::endl;
    }

    //5. Make OpenGL ES the current API.
    eglBindAPI(EGL_OPENGL_API);

    //6. Create a context.
    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
    if (context == EGL_NO_CONTEXT) {
        std::cerr << "Failed to create EGL Context." << std::endl
            << "Error: " << eglGetError() << std::endl;
    }
    else {
        std::cerr << "Successfully created OpenGL ES Context." << std::endl;
    }

    //7. Bind context to to the current thread.
    bool result = eglMakeCurrent(display, surface, surface, context);
    if (!result) {
        std::cerr << "Failed to make the egl context to the current one." << std::endl;
        eglTerminate(display);
        return EXIT_FAILURE;
    }

    //8. Load shaders
    GLuint program = load_shaders(argv[1], argv[2]);
    if (!program) {
        std::cerr << "Failed to create shader program. See above for more details" << std::endl;
        eglTerminate(display);
        return EXIT_FAILURE;
    }

    // 9. Draw
    // Getting location of our uniform variables
    GLuint texture_loc = glGetUniformLocation(program, "texture");
    GLuint width_loc = glGetUniformLocation(program, "width");
    GLuint height_loc = glGetUniformLocation(program, "height");

    // Create a FBO that will allow us to do offscreen rendering
    GLuint fbo_render_texture;
    GLuint fbo = init_fbo((int)image_width, (int)image_height, fbo_render_texture);
    if (!fbo)
    {
        eglTerminate(display);
        return EXIT_FAILURE;
    }

    // Create texture from image data
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_width, image_height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, image_texture);
    glUniform1i(texture_loc, 0);

    glUniform1i(width_loc, image_width);
    glUniform1i(height_loc, image_height);

    // Create fullscreen quad to trigger rasterisation (use of vertex and fragment shaders)
    Quad q;
    q.init();
    q.display(program);

    // Once rasterisation is done, data have been generated and it is now possible to transfer them back from VRAM to memory.
    unsigned char* data = new unsigned char[image_width * image_height * 3];
    glReadPixels(0, 0, image_width, image_height, GL_RGB, GL_UNSIGNED_BYTE, data);

    // Switching back to our classic buffer with scene rendered in FBO render texture
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Save image (optional)
    client.setCameraFrame(new CameraFrame(image_width, image_height, 3, data), true);
    write_ppm(argv[3], data, image_width, image_height);

    //10. Clean
    glDeleteTextures(1, &image_texture);
    glDeleteProgram(program);
    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(display, surface);
    eglDestroyContext(display, context);
    eglTerminate(display);
    delete_fbo(fbo, fbo_render_texture);

    return EXIT_SUCCESS;
}
