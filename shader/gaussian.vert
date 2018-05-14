#version 130

attribute vec2 vtx_position;

void main() {
    gl_Position = vec4(vtx_position, 0.0, 1.0);
}
