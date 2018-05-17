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
    vec3 color = texture2D(texture, texcoord).rgb;

    // Since we are between -1 and 1, moving from one pixel to another requires custom shifting
    float hstep = 1.0/height;
    float wstep = 1.0/width;
    vec2 left  = texcoord - vec2(wstep, 0);
    vec2 right = texcoord + vec2(wstep, 0);
    vec2 top   = texcoord - vec2(0, hstep);
    vec2 bot   = texcoord + vec2(0, hstep);
    vec2 ltop  = texcoord - vec2(wstep, hstep);
    vec2 rtop  = texcoord + vec2(wstep, -hstep);
    vec2 lbot  = texcoord - vec2(wstep, -hstep);
    vec2 rbot  = texcoord + vec2(wstep, hstep);

    // We convert image to GRAY by summing the 3 channels & dividing by 3.
    // vec3(a, vec3(1)) will sum a since dot product is vec1.x * vec2.x + vec1.y * vec2.y ...
    vec3 left_color  = vec3(dot(texture2D(texture, left).rgb, vec3(1)))/3.0;
    vec3 right_color = vec3(dot(texture2D(texture, right).rgb, vec3(1)))/3.0;
    vec3 top_color   = vec3(dot(texture2D(texture, top).rgb, vec3(1)))/3.0;
    vec3 bot_color   = vec3(dot(texture2D(texture, bot).rgb, vec3(1)))/3.0;
    vec3 ltop_color  = vec3(dot(texture2D(texture, ltop).rgb, vec3(1)))/3.0;
    vec3 rtop_color  = vec3(dot(texture2D(texture, rtop).rgb, vec3(1)))/3.0;
    vec3 lbot_color  = vec3(dot(texture2D(texture, lbot).rgb, vec3(1)))/3.0;
    vec3 rbot_color  = vec3(dot(texture2D(texture, rbot).rgb, vec3(1)))/3.0;

    vec3 sobel_h = rtop_color + 2.0 * right_color + rbot_color - (ltop_color + 2.0 * left_color + lbot_color);
    vec3 sobel_v = ltop_color + 2.0 * top_color + rtop_color - (lbot_color + 2.0 * bot_color + rbot_color);
    vec3 sobel = sqrt(sobel_h * sobel_h + sobel_v * sobel_v);

    gl_FragColor = vec4(1.0 - sobel, 1.0);
}
