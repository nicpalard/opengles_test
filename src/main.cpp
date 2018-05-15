#include "quad.hpp"
#include "gles_utils.hpp"
#include "ppm_utils.hpp"

int main(int argc, char** argv)
{
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <vertex shader path> <fragment shader path> <image path>" << std::endl;
        return EXIT_FAILURE;
    }

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
    EGLint pbufferAttributes[] = {EGL_WIDTH, 1080, EGL_HEIGHT, 720, EGL_NONE};
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

    GLuint program = load_shaders(argv[1], argv[2]);
    if (!program) {
        std::cerr << "Failed to create shader program. See above for more details" << std::endl;
        eglTerminate(display);
        return EXIT_FAILURE;
    }
    // Getting location of our uniform variables
    GLuint texture_loc = glGetAttribLocation(program, "texture");
    GLuint width_loc = glGetAttribLocation(program, "width");
    GLuint height_loc = glGetAttribLocation(program, "height");

    // Initialize screen quad
    Quad q; q.init();
    
    /* Drawing part */
    GLuint fbo_render_texture;
    GLuint fbo = init_fbo(1080, 720, fbo_render_texture);
    if (!fbo)
    {
        eglTerminate(display);
        return EXIT_FAILURE;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Switching back to our classic buffer with scene rendered in FBO render texture
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Now we can use our post processing shader since we have our scene in a texture.
    glUseProgram(program);
    glBindTexture(GL_TEXTURE_2D, fbo_render_texture);
    // Since we just binded the texture, the id is 0
    glUniform1i(texture_loc, GL_TEXTURE0);
    glUniform1i(width_loc, 1080);
    glUniform1i(height_loc, 720);

    q.display(program);

    delete_fbo(fbo, fbo_render_texture);
}