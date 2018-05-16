#version 130

#ifdef GL_ES
precision mediump float
#endif

uniform int width;
uniform int height;
uniform sampler2D texture;

void main() {
    vec2 texcoord = vec2(gl_FragCoord.x/width, gl_FragCoord.y/height);
    vec3 color = texture2D(texture, texcoord).rgb;

    // Since we are between -1 and 1, moving from one pixel to another requires new units
    float hstep = 1/height;
    float wstep = 1/width;
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

    vec3 sum = left_color + right_color + 
            top_color + bot_color + 
            ltop_color + rtop_color + 
            lbot_color + rbot_color + 
            color;

    gl_FragColor = vec4(sum/9, 1.0);
}
