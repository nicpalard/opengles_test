#include <iomanip>
#include <chrono>
#include <fstream>

#include "quad.hpp"
#include "gles_utils.hpp"
#include "ImageUtils.hpp"


int main(int argc, char** argv)
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <vertex shader path> <fragment shader path>" << std::endl;
        return EXIT_FAILURE;
    }

    std::fstream csvfile;
    csvfile.open("bench.csv", std::fstream::in | std::fstream::out | std::fstream::app);
    if (!csvfile.is_open())
    {
        std::cerr << "Could not opencv bench.csv, benchmark results are not going to be saved" << std::endl;
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
    EGLint pbufferAttributes[] = {EGL_WIDTH, 10000, EGL_HEIGHT, 10000, EGL_NONE};
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

    //--------------- BENCH SETUP ----------------
    std::cout << std::endl
                << "** Starting benchmark **" << std::endl
                << "Convolution using " << argv[2] << std::endl
                << "---------------------------------------------" << std::endl
                << "Size\t\tSize (MB)\tCTime (ms)\tTTime (ms)\tTotal (ms)\tBandwidth (MB/s)" << std::endl
                << std::fixed << std::setprecision(3) << std::setfill('0');

    if (csvfile.is_open())
        csvfile << "Size,Size (MB),Compute Time (ms),Transfer Time (ms),Total Time (ms),Bandwidth (MB/s)" << std::endl;

    typedef std::chrono::high_resolution_clock Time;
    typedef std::chrono::duration<double> fsec;
    typedef std::chrono::milliseconds ms;
    //--------------- BENCH SETUP ----------------

    GLuint fbo, fbo_render_texture, image_texture;
    for (int N = 128 ; N <= 8192 ; N+=N)
    {
        int image_width = N;
        int image_height = N;
        int size = image_width * image_height * 3;
        float* image = generate_random_image(image_width, image_height, 3);

        double total_time = 0, transfer_time = 0, render_time = 0;
        int iterations = 1;
        for (int i = 0 ; i < iterations ; ++i)
        {
            //--------------- BENCH TOTAL TIME ----------------
            auto total_start = Time::now();

            // 9. Draw
            // Getting location of our uniform variables
            GLuint texture_loc = glGetUniformLocation(program, "texture");
            GLuint width_loc = glGetUniformLocation(program, "width");
            GLuint height_loc = glGetUniformLocation(program, "height");


            // Create a FBO that will allow us to do offscreen rendering
            fbo = init_fbo((int)image_width, (int)image_height, fbo_render_texture);
            if (!fbo)
            {
                eglTerminate(display);
                return EXIT_FAILURE;
            }

            // Create texture from image data
            glGenTextures(1, &image_texture);
            glBindTexture(GL_TEXTURE_2D, image_texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

            //--------------- BENCH TRANSFER TIME ----------------
            auto transfer_start = Time::now();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_width, image_height, 0, GL_RGB, GL_FLOAT, image);

            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glClearColor(0.0, 0.0, 0.0, 1.0);
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(program);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, image_texture);
            glUniform1i(texture_loc, 0);

            glUniform1i(width_loc, image_width);
            glUniform1i(height_loc, image_height);

            //--------------- BENCH COMPUTE TIME ----------------
            auto render_start = Time::now();

            // Create fullscreen quad to trigger rasterisation (use of vertex and fragment shaders)
            Quad q;
            q.init();
            q.display(program);

            auto render_end = Time::now();
            render_time += fsec(render_end - render_start).count();
            //--------------- BENCH COMPUTE TIME ----------------

            // Once rasterisation is done, data have been generated and it is now possible to transfer them back from VRAM to memory.
            float* data = new float[image_width * image_height * 3];
            glReadPixels(0, 0, image_width, image_height, GL_RGB, GL_FLOAT, data);

            auto transfer_end = Time::now();
            transfer_time += fsec(transfer_end - transfer_start).count();
            //--------------- BENCH TRANSFER TIME ----------------

            auto total_end = Time::now();
            total_time += fsec(total_end - total_start).count();
            //--------------- BENCH TOTAL TIME ----------------

            // Switching back to our classic buffer with scene rendered in FBO render texture
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

        }

        double realsize = (size * sizeof(GL_FLOAT)) / (double)1e6;
        double ms_transfer_time = transfer_time * 1e3 / (double)iterations;
        double ms_render_time   = render_time   * 1e3 / (double)iterations;
        double ms_total_time    = total_time    * 1e3 / (double)iterations;
        double bandwidth = realsize / (ms_total_time / 1e3);

        std::cout << image_width << " x " << image_height << "\t"
                  << realsize << "\t\t"
                  << ms_render_time << "\t\t"
                  << ms_transfer_time << "\t\t"
                  << ms_total_time << "\t\t"
                  << bandwidth << std::endl;

        if (csvfile.is_open())
        {
            csvfile << image_width << "x" << image_height << ","
                    << realsize << ","
                    << ms_render_time << ","
                    << ms_transfer_time << ","
                    << ms_total_time << ","
                    << bandwidth << std::endl;
        }
    }

    //10. Clean
    glDeleteTextures(1, &image_texture);
    glDeleteProgram(program);
    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(display, surface);
    eglDestroyContext(display, context);
    eglTerminate(display);
    delete_fbo(fbo, fbo_render_texture);

    if (csvfile.is_open())
        csvfile.close();

    return EXIT_SUCCESS;
}
