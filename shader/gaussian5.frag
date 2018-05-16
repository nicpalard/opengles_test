#version 130

/**
    Gaussian Blur Fragment Shader
    GLSL implementation of a 5x5 gaussian blur without any control
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

    vec2 left   = texcoord - vec2(wstep,       0);
    vec2 lleft  = texcoord - vec2(2.0 * wstep, 0);

    vec2 right  = texcoord + vec2(wstep,       0);
    vec2 rright = texcoord + vec2(2.0 * wstep, 0);

    vec2 top    = texcoord - vec2(0, hstep);
    vec2 ttop   = texcoord - vec2(0, 2.0 * hstep);

    vec2 bot    = texcoord + vec2(0, hstep);
    vec2 bbot   = texcoord + vec2(0, 2.0 * hstep);

    vec2 ltop   = texcoord - vec2(wstep,       hstep);
    vec2 lttop  = texcoord - vec2(wstep,       2.0 * hstep);
    vec2 lltop  = texcoord - vec2(2.0 * wstep, hstep);
    vec2 llttop = texcoord - vec2(2.0 * wstep, 2.0 * hstep);

    vec2 rtop   = texcoord + vec2(wstep,       -hstep);
    vec2 rttop  = texcoord + vec2(wstep,       -2.0 * hstep);
    vec2 rrtop  = texcoord + vec2(2.0 * wstep, -hstep);
    vec2 rrttop = texcoord + vec2(2.0 * wstep, - 2.0 * hstep);

    vec2 lbot    = texcoord - vec2(wstep,       -hstep);
    vec2 lbbot   = texcoord - vec2(wstep,       -2.0 * hstep);
    vec2 llbot   = texcoord - vec2(2.0 * wstep, -hstep);
    vec2 llbbot  = texcoord - vec2(2.0 * wstep, -2.0 * hstep);

    vec2 rbot    = texcoord + vec2(wstep,       hstep);
    vec2 rbbot   = texcoord + vec2(wstep,       2.0 * hstep);
    vec2 rrbot   = texcoord + vec2(2.0 * wstep, hstep);
    vec2 rrbbot  = texcoord + vec2(2.0 * wstep, 2.0 * hstep);

    vec3 left_color   = texture2D(texture, left).rgb;
    vec3 lleft_color  = texture2D(texture, lleft).rgb;

    vec3 right_color  = texture2D(texture, right).rgb;
    vec3 rright_color = texture2D(texture, rright).rgb;

    vec3 top_color    = texture2D(texture, top).rgb;
    vec3 ttop_color   = texture2D(texture, ttop).rgb;

    vec3 bot_color    = texture2D(texture, bot).rgb;
    vec3 bbot_color   = texture2D(texture, bbot).rgb;

    vec3 ltop_color    = texture2D(texture, ltop).rgb;
    vec3 lttop_color    = texture2D(texture, lttop).rgb;
    vec3 lltop_color   = texture2D(texture, lltop).rgb;
    vec3 llttop_color  = texture2D(texture, llttop).rgb;

    vec3 rtop_color    = texture2D(texture, rtop).rgb;
    vec3 rttop_color    = texture2D(texture, rttop).rgb;
    vec3 rrtop_color   = texture2D(texture, rrtop).rgb;
    vec3 rrttop_color  = texture2D(texture, rrttop).rgb;

    vec3 lbot_color    = texture2D(texture, lbot).rgb;
    vec3 lbbot_color    = texture2D(texture, lbbot).rgb;
    vec3 llbot_color   = texture2D(texture, llbot).rgb;
    vec3 llbbot_color  = texture2D(texture, llbbot).rgb;
    
    vec3 rbot_color    = texture2D(texture, rbot).rgb;
    vec3 rbbot_color    = texture2D(texture, rbbot).rgb;
    vec3 rrbot_color   = texture2D(texture, rrbot).rgb;
    vec3 rrbbot_color  = texture2D(texture, rrbbot).rgb;

    vec3 sum =  llttop_color * 0.031827 + lttop_color * 0.037541 + ttop_color * 0.039665 + rttop_color * 0.037541 + rrttop_color * 	0.031827 +
                lltop_color  * 0.037541 + ltop_color  * 0.044281 + top_color  * 0.046787 + rtop_color  * 0.044281 + rrtop_color  * 0.037541 +
                lleft_color  * 0.039665 + left_color  * 0.046787 + color      * 0.049434 + right_color * 0.049434 + rright_color * 0.039665 +
                llbot_color  * 0.031827 + lbot_color  * 0.037541 + bot_color  * 0.039665 + rbot_color  * 0.037541 + rrbot_color  * 0.031827 +
                llbbot_color * 0.037541 + lbbot_color * 0.044281 + bbot_color * 0.046787 + rbbot_color * 0.044281 + rrbbot_color * 0.037541;
    gl_FragColor = vec4(sum, 1.0);
}
