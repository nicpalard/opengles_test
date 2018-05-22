#include <hiredis/hiredis.h>

#include "quad.hpp"
#include "gles_utils.hpp"
#include "image_utils.hpp"
#include "redis_utils.hpp"

int main(int argc, char** argv)
{
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <vertex shader path> <fragment shader path> <output file>" << std::endl;
        return EXIT_FAILURE;
    }

    //0. Prepare image texture
    // Getting image from redis server hosted on 127.0.0.1:6379
    // Image access key is: nectar:jiii-mi:camera-server:camera#0
    // Image format is RGBA
    // There is 27 extra bytes in the buffer
    int image_width = 640, image_height = 480;
    redisContext *c = redis_connect("127.0.0.1", 6379, true);
    if ( c == NULL || c->err) { std::cerr << "Context error: " << c->errstr << std::endl; return EXIT_FAILURE;}
    unsigned char* image = redis_get_image(c, "nectar:jiii-mi:camera-server:camera#0", image_width, image_height);

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
    write_ppm(argv[3], data, image_width, image_height);
    redis_set_image(c, "image:proc:output", data, image_width, image_height, 3);

    //10. Clean
    glDeleteTextures(1, &image_texture);
    glDeleteProgram(program);
    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(display, surface);
    eglDestroyContext(display, context);
    eglTerminate(display);
    delete_fbo(fbo, fbo_render_texture);

    redisFree(c);

    return EXIT_SUCCESS;
}
