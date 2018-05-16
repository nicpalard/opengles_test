#ifndef _QUAD_H_
#define _QUAD_H_

#include <GLES2/gl2.h>
#include <EGL/egl.h>

class Quad
{
public:
    Quad();
    ~Quad();

    void init();
    void setPos(unsigned int, float, float);
    void display(GLuint);

private:
    float mPos[8];
    int mIndices[6];
    GLuint vbo;
};

#endif