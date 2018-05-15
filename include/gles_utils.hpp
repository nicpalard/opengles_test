#ifndef _GLES_UTILS_H_
#define _GLES_UTILS_H_

#include <GLES2/gl2.h>
#include <EGL/egl.h>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <streambuf>

const EGLint EGL_CONFIG_ATTRIBUTES[] = {
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

const EGLint contextAttribs[] = {
  EGL_CONTEXT_CLIENT_VERSION, 2,
  EGL_NONE
};

inline bool check(GLuint object, GLenum to_check,
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

inline GLuint load_shaders(std::string vertex_shader_path,
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
          return 0;
    }
    else {
          std::cerr << "Successfully linked program." << std::endl;
    }

    return program;
}

inline int init_fbo(int width, int height, GLuint fbo_render_texture)
{
    //1. Generate Frame Buffer Object
    GLuint fboId;
    glGenFramebuffers(1, &fboId);
    //2. Bind it
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);

    //3. Generate texture
    glGenTextures(1, &fbo_render_texture);
    //4. Bind it
    glBindTexture(GL_TEXTURE_2D, fbo_render_texture);
    //5. Set texture properties
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    //6. Attach texture to FBO color attachment
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_render_texture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Failed to construct FBO." << std::endl;
        return 0;
    }

    //7. Switch back to original texture & framebuffer
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return fboId;
}

inline void delete_fbo(GLuint fbo, GLuint fbo_render_texture)
{
    glDeleteTextures(1, &fbo_render_texture);
    glDeleteFramebuffers(1, &fbo);
}

inline void bla() {
}

#endif