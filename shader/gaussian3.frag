#version 130

/**
    Gaussian Blur Fragment Shader
    GLSL implementation of a 3x3 gaussian blur without any control
*/

#ifdef GL_ES
precision mediump float
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

    vec3 left_color  = texture2D(texture, left).rgb;
    vec3 right_color = texture2D(texture, right).rgb;;
    vec3 top_color   = texture2D(texture, top).rgb;
    vec3 bot_color   = texture2D(texture, bot).rgb;
    vec3 ltop_color  = texture2D(texture, ltop).rgb;
    vec3 rtop_color  = texture2D(texture, rtop).rgb;
    vec3 lbot_color  = texture2D(texture, lbot).rgb;
    vec3 rbot_color  = texture2D(texture, rbot).rgb;

    vec3 sum =  ltop_color * 0.111018 + top_color * 0.111157 + rtop_color  * 0.111018 +
                left_color * 0.111157 + color     * 0.111296 + right_color * 0.111157 +
                lbot_color * 0.111018 + bot_color * 0.111157 + rbot_color  * 0.111018;
    gl_FragColor = vec4(sum, 1.0);
}
