#include "quad.hpp"

#include <iostream>

using namespace std;

Quad::Quad()
{
    setPos     (0,-1.f,-1.f);
    setPos     (1, 1.f,-1.f);
    setPos     (2, 1.f, 1.f);
    setPos     (3,-1.f, 1.f);
}

Quad::~Quad()
{
    glDeleteBuffers(1, &vbo);
}

void Quad::init()
{
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, mPos, GL_STATIC_DRAW);
}

void Quad::setPos(unsigned int id, float x, float y)
{
    if (id > 3)
        cerr << "Warning: Invalid vertex coordinates" << endl;

    mPos[2*id  ] = x;
    mPos[2*id+1] = y;
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
    glDrawElements(GL_TRIANGLES, 6, GL_FLOAT, 0);
    if (vertex_loc >= 0)
    {
        glDisableVertexAttribArray(vertex_loc);
    }
}
