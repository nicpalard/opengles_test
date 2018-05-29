#version 130

/**
    Sobel Edge Detection Fragment Shader
    GLSL implementation of the sobel edge detection
*/

#ifdef GL_ES
precision mediump float;
#endif

uniform int width;
uniform int height;
uniform sampler2D texture;

void main() {
    vec2 texcoord = vec2(gl_FragCoord.x/width, gl_FragCoord.y/height);
    float color = texture2D(texture, texcoord).r;


    gl_FragColor = vec4(color, color, color, 1.0);
}
