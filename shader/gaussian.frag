#version 130

uniform int width;
uniform int height;
uniform sampler2D texture;

void main() {
    vec2 texcoord = vec2(gl_FragCoord.x/width, gl_FragCoord.y/height);
    vec4 texture_value = texture2D(texture, texcoord);
    gl_FragColor = texture_value.bgra;
}
