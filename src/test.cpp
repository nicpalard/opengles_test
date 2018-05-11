#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <IL/il.h>

#include <iostream>
#include <vector>

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

static bool check(GLuint object, GLenum to_check, void (*glGet__iv)(GLuint, GLenum, GLint*), void (*glGet__infoLog)(GLuint, GLsizei, GLsizei*, GLchar*), void (*glDelete__)(GLuint))
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

int main(int argc, char** argv)
{
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
  GLuint program = glCreateProgram();
  glUseProgram(program);

  const char* vertex_shader_code = STRINGIFY(
    attribute vec2 texcoord;
	varying vec2 o_texcoord;

	void main(void)
	{
	  o_texcoord = texcoord;
	  gl_Position = vec4(texcoord, 1.0, 1.0);
	}
  );

  // Create sub shader: vertex shader
  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_code, NULL);
  glCompileShader(vertex_shader);
  if (!check(vertex_shader, GL_COMPILE_STATUS, glGetShaderiv, glGetShaderInfoLog, glDeleteShader)) {
	std::cerr << "Failed to compile vertex shader." << std::endl;
	eglTerminate(display);
	return EXIT_FAILURE;
  }
  else {
	std::cerr << "Successfully compilated vertex shader." << std::endl;
  }

  const char* fragment_shader_code = STRINGIFY(
	varying vec2 o_texcoord;
	uniform sampler2D texture;
	uniform vec2 tex_size;

	void main(void)
	{
	  vec4 value = texture2D(texture, o_texcoord);
	  gl_FragColor = vec4(1.0, 0.0, 0.0, 0.0);
	}
  );

  // Create sub shader: fragment shader
  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_code, NULL);
  glCompileShader(fragment_shader);
  if (!check(fragment_shader, GL_COMPILE_STATUS, glGetShaderiv, glGetShaderInfoLog, glDeleteShader)) {
	std::cerr << "Failed to compile fragment shader." << std::endl;
	eglTerminate(display);
	return EXIT_FAILURE;
  }
  else {
	std::cerr << "Successfully compilated fragment shader." << std::endl;
  }

  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  if (!check(program, GL_LINK_STATUS, glGetProgramiv, glGetProgramInfoLog, glDeleteProgram)) {
	std::cerr << "Failed to link program." << std::endl;
	eglTerminate(display);
	return EXIT_FAILURE;
  }
  else {
	std::cerr << "Successfully linked program." << std::endl;
  }

  /* Initialization of DevIL */
  if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION) {
	std::cerr << "Failed to use DevIL: Wrong version." << std::endl;
	return EXIT_FAILURE;
  }
  ilInit(); 
  ILuint image = load_image(argv[1]);
  GLuint texId;
  glGenTextures(1, &texId); /* Texture name generation */
  glBindTexture(GL_TEXTURE_2D, texId); /* Binding of texture name */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); /* We will use linear interpolation for magnification filter */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); /* We will use linear interpolation for minifying filter */
  glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 
			   0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData()); /* Texture specification */

  
  glUseProgram(program);

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  GLuint texcoord_loc = glGetAttribLocation(program, "texcoord");
  
  GLuint tex_loc = glGetUniformLocation(program, "texture");
  glUniform1f(tex_loc, texId);

  GLuint tex_size_loc = glGetUniformLocation(program, "tex_size");
  glUniform2f(tex_size_loc, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT));

  // Cleanup
  /* Delete used resources and quit */
  ilDeleteImages(1, &image); /* Because we have already copied image data into texture data we can release memory used by image. */
  glDeleteTextures(1, &texId);
  
  eglDestroyContext(display, context);
  eglDestroySurface(display, surface);
  eglTerminate(display);
  return EXIT_SUCCESS;
}
