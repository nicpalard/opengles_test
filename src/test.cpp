#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <IL/il.h>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <streambuf>

#define STRINGIFY(x) #x

static const int WIDTH = 800;
static const int HEIGHT = 600;

static const EGLint configAttribs[] = {
  EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
  EGL_BLUE_SIZE, 8,
  EGL_GREEN_SIZE, 8,
  EGL_RED_SIZE, 8,
  EGL_DEPTH_SIZE, 8,

  // Uncomment the following to enable MSAA 
  //EGL_SAMPLE_BUFFERS, 1, // <-- Must be set to 1 to enable multisampling!
  //EGL_SAMPLES, 4, // <-- Number of samples

  // Uncomment the following to enable stencil buffer
  //EGL_STENCIL_SIZE, 1,

  EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
  EGL_NONE
};

static const EGLint pbufferAttribs[] = {
  EGL_WIDTH, WIDTH,
  EGL_HEIGHT, HEIGHT,
  EGL_NONE
};

static const EGLint contextAttribs[] = {
  EGL_CONTEXT_CLIENT_VERSION, 2,
  EGL_NONE
};

static bool check(GLuint object, GLenum to_check,
                  void (*glGet__iv)(GLuint, GLenum, GLint*),
                  void (*glGet__infoLog)(GLuint, GLsizei, GLsizei*, GLchar*),
                  void (*glDelete__)(GLuint))
{
  GLint success = 0;
  glGet__iv(object, to_check, &success);
  if (success == GL_FALSE) {
	GLint max_length = 0;
	glGet__iv(object, GL_INFO_LOG_LENGTH, &max_length);

	char* error_log = new char[max_length];
	glGet__infoLog(object, max_length, NULL, error_log);

	std::cerr << "Error while compiling:" << std::endl << error_log;
	glDelete__(object); // Don't leak the object.
	free(error_log); // Don't leak the error log.
	return false;
  }
  return true;
}

static GLuint load_shaders(std::string vertex_shader_path,
                           std::string fragment_shader_path)
{
    // Create Vertex shader
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    // Loading source
    std::ifstream vertex_shader_file(vertex_shader_path);
    std::string vertex_shader_code((std::istreambuf_iterator<char>(vertex_shader_file)),
                     std::istreambuf_iterator<char>());
    const char* vertex_code = vertex_shader_code.c_str();

    glShaderSource(vertex_shader, 1, &vertex_code, NULL);
    glCompileShader(vertex_shader);
    if (!check(vertex_shader, GL_COMPILE_STATUS, glGetShaderiv, glGetShaderInfoLog, glDeleteShader)) {
          std::cerr << "Failed to compile vertex shader." << std::endl;
          return -1;
    }
    else {
          std::cerr << "Successfully compilated vertex shader." << std::endl;
    }

    // Create Fragment shader
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    // Loading source
    std::ifstream fragment_shader_file(fragment_shader_path);
    std::string fragment_shader_code((std::istreambuf_iterator<char>(fragment_shader_file)),
                     std::istreambuf_iterator<char>());
    const char* fragment_code = fragment_shader_code.c_str();

    glShaderSource(fragment_shader, 1, &fragment_code, NULL);
    glCompileShader(fragment_shader);
    if (!check(fragment_shader, GL_COMPILE_STATUS, glGetShaderiv, glGetShaderInfoLog, glDeleteShader)) {
          std::cerr << "Failed to compile fragment shader." << std::endl;
          return -1;
    }
    else {
          std::cerr << "Successfully compilated fragment shader." << std::endl;
    }

    // Create a shader program
    GLuint program = glCreateProgram();
    glUseProgram(program);
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    if (!check(program, GL_LINK_STATUS, glGetProgramiv, glGetProgramInfoLog, glDeleteProgram)) {
          std::cerr << "Failed to link program." << std::endl;
          return -1;
    }
    else {
          std::cerr << "Successfully linked program." << std::endl;
    }

    return program;
}


static int load_image(char* filename)
{
  ILboolean success;
  ILuint image;
  ilGenImages(1, &image);
  ilBindImage(image);
  success = ilLoadImage(filename);
  if (!success) {
	std::cerr << "Failed to load image " << filename << "." << std::endl;
	return -1;
  }
  success = ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
  if (!success) {
	std::cerr << "Failed to convert image to uchar*" << std::endl;
	return -1;
  }

  return image;
}

GLuint init_quad()
{
    // Generating Vertex Buffer Objects
    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);

    static const float vertices[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f,  1.0f,
        -1.0f,  1.0f,
        1.0f, -1.0f,
        1.0f,  1.0f,
    };
    // Binding first (and only) buffer vertices & transfering data to VRAM
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, vertices, GL_STATIC_DRAW);

    return vertex_buffer;

}

void display_quad(GLuint program, GLuint vertex_buffer)
{
    GLuint vertex_loc = glGetAttribLocation(program, "vtx_position");
    if (vertex_loc >= 0)
    {
        std::cerr << "Segfault 01" << std::endl;
        glEnableVertexAttribArray(vertex_loc);
        std::cerr << "Segfault 03" << std::endl;
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        std::cerr << "Segfault 02" << std::endl;
        glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }
    std::cerr << "Segfault 04" << std::endl;
    glDrawElements(GL_TRIANGLES, 6, GL_FLOAT, 0);
    std::cerr << "Segfault 05" << std::endl;
    if (vertex_loc >= 0)
    {
        glDisableVertexAttribArray(vertex_loc);
    }
}


int main(int argc, char** argv)
{

    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <vertex shader path> <fragment shader path> <image path>" << std::endl;
        return EXIT_FAILURE;
    }

  // Get an EGL valid display
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

  
  // Create a connection to the display
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

  // OpenGL ES Config are used to specify things like multi sampling, channel size, stencil buffer usage, & more
  // See the doc: https://www.khronos.org/registry/EGL/sdk/docs/man/html/eglChooseConfig.xhtml for more informations
  EGLConfig config;
  EGLint num_configs; 
  if (!eglChooseConfig(display, configAttribs, &config, 1, &num_configs)) {
	std::cerr << "Failed to choose EGL Config" << std::endl
			  << "Error: " << eglGetError() << std::endl;
	eglTerminate(display);
	return EXIT_FAILURE;
  }
  else {
	std::cerr << "Successfully choose OpenGL ES Config ("<< num_configs << ")." << std::endl;
  }

  // Creating an OpenGL Render Surface with surface attributes defined above.
  EGLSurface surface = eglCreatePbufferSurface(display, config, pbufferAttribs);
  if (surface == EGL_NO_SURFACE) {
	std::cerr << "Failed to create EGL Surface." << std::endl
			  << "Error: " << eglGetError() << std::endl;
  }
  else {
	std::cerr << "Successfully created OpenGL ES Surface." << std::endl;
  }

  eglBindAPI(EGL_OPENGL_API);
  EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
  if (context == EGL_NO_CONTEXT) {
	std::cerr << "Failed to create EGL Context." << std::endl
			  << "Error: " << eglGetError() << std::endl;
  }
  else {
	std::cerr << "Successfully created OpenGL ES Context." << std::endl;
  }

  //Bind context to surface
  eglMakeCurrent(display, surface, surface, context);

  // Create viewport and check if it has been created correctly
  glViewport(0, 0, WIDTH, HEIGHT);
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  if (viewport[2] != WIDTH || viewport[3] != HEIGHT) {
	std::cerr << "Failed to create the viewport. Size does not match (glViewport/glGetIntegerv not working)." << std::endl
			  << "OpenGL ES might be faulty!" << std::endl
			  << "If you are on Raspberry Pi, you should not updated EGL as it will install fake EGL." << std::endl;
	eglTerminate(display);
	return EXIT_FAILURE;
  }

  // Clear buffer and get ready to draw some things
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Create a shader program
  GLuint program = load_shaders(std::string(argv[1]), std::string(argv[2]));
  if (program == -1)
  {
      std::cerr << "Failed to create a shader program. See above for more details." << std::endl;
      eglTerminate(display);
      return EXIT_FAILURE;
  }

  /* Initialization of DevIL */
  if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION) {
	std::cerr << "Failed to use DevIL: Wrong version." << std::endl;
	return EXIT_FAILURE;
  }

  ilInit(); 
  ILuint image = load_image(argv[3]);
  GLuint texId;
  glGenTextures(1, &texId); /* Texture name generation */
  glBindTexture(GL_TEXTURE_2D, texId); /* Binding of texture name */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); /* We will use linear interpolation for magnification filter */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); /* We will use linear interpolation for minifying filter */
  glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 
			   0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData()); /* Texture specification */


  GLuint vertex_buffer = init_quad();

  //1. Generate FBO and bind it.
  GLuint framebufferId;
  glGenFramebuffers(1, &framebufferId);
  glBindFramebuffer(GL_FRAMEBUFFER, framebufferId);

  //2. Init texture
  GLuint renderedTeture;
  glGenTextures(1, &renderedTeture);
  //3. Create texture as an empty image
  glBindTexture(GL_TEXTURE_2D, renderedTeture);
  glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0,GL_RGB, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  //4. Attach the texture to FBO color attachment
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTeture, 0);
  // Always check that our framebuffer is ok
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
      std::cerr << "Failed to construct FBO" << std::endl;
      return false;
  }

  //8. switch back to original framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  /*

  // The fullscreen quad's FBO
  GLuint quad_VertexArrayID;
  glGenVertexArrays(1, &quad_VertexArrayID);
  glBindVertexArray(quad_VertexArrayID);

  static const GLfloat vertices_data[] = {
      -1.0f, -1.0f, 0.0f,
      1.0f, -1.0f, 0.0f,
      -1.0f,  1.0f, 0.0f,
      -1.0f,  1.0f, 0.0f,
      1.0f, -1.0f, 0.0f,
      1.0f,  1.0f, 0.0f,
  };

  GLuint quad_vertexbuffer;
  glGenBuffers(1, &quad_vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), vertices_data, GL_STATIC_DRAW);


  do {
      glUseProgram(program);

      GLuint texture_loc = glGetUniformLocation(program, "texture");
      GLuint width_loc = glGetUniformLocation(program, "width");
      GLuint height_loc = glGetUniformLocation(program, "height");

      glUniform1i(texture_loc, texId);
      glUniform1i(width_loc, ilGetInteger(IL_IMAGE_WIDTH));
      glUniform1i(height_loc, ilGetInteger(IL_IMAGE_HEIGHT));

      GLuint vertex_loc = glGetAttribLocation(program, "vtx_position");
      glEnableVertexAttribArray(vertex_loc);
      glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
      glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
      glDrawArrays(GL_TRIANGLES, 0, 6);

      glDisableVertexAttribArray(vertex_loc);

      // Render to our framebuffer
      glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
      glViewport(0, 0, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT));
  } while (true);
  */


  display_quad(program, vertex_buffer);

  // Cleanup
  /* Delete used resources and quit */
  ilDeleteImages(1, &image); /* Because we have already copied image data into texture data we can release memory used by image. */
  glDeleteTextures(1, &texId);
  
  eglDestroyContext(display, context);
  eglDestroySurface(display, surface);
  eglTerminate(display);
  return EXIT_SUCCESS;
}
