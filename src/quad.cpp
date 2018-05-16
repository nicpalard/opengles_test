#include "quad.hpp"

#include <iostream>

using namespace std;

Quad::Quad()
{
    set_position(0,-1.f,-1.f);
    set_position(1,-1.f, 1.f);
    set_position(2, 1.f,-1.f);
    set_position(3, 1.f, 1.f);
}

Quad::~Quad()
{
    glDeleteBuffers(1, &vbo);
}

void Quad::init()
{
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, m_position, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Quad::set_position(unsigned int id, float x, float y)
{
    if (id > 3)
        cerr << "Warning: Invalid vertex coordinates" << endl;

    m_position[2*id  ] = x;
    m_position[2*id+1] = y;
}


void Quad::display(GLuint program)
{
    GLuint vertex_loc = glGetAttribLocation(program, "vtx_position");
    if (vertex_loc >= 0)
    {
        glEnableVertexAttribArray(vertex_loc);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(vertex_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if (vertex_loc >= 0)
    {
        glDisableVertexAttribArray(vertex_loc);
    }

}
