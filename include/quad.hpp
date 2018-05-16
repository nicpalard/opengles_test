#ifndef _QUAD_H_
#define _QUAD_H_

#include <GLES2/gl2.h>
#include <EGL/egl.h>

/**
    Helper class to manage creation & rendering of simple full screen quads
**/
class Quad
{
public:
    Quad();
    ~Quad();

    /**
        Initialize quad's buffers.
    */
    void init();

    /**
        Set the quad's position.
        @param index the ID of the created vertex
        @param x the x position of the vertex
        @param y the y position of the vertex
    */
    void set_position(unsigned int index, float x, float y);

    /**
        Display the quad to the current buffer.
        @param shader the shader program that the quad should use.
    **/
    void display(GLuint shader);

private:
    float m_position[8];; // The vertices (x,y) positions
    GLuint vbo; // The Vertex Buffer Object that stores vertices, texcoords, normals. Here it only stores vertices
};

#endif