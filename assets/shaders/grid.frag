#version 450

// #include "defines/frag_def.h"
#include "defines/defines.h"
#include "defines/helpers.h"

layout (location = 0) in vec3 innear;
layout(location = 1) in vec3 infar;
layout (location = 0) out vec4 out_frag_color;

vec4 grid(vec3 pos, float scale, float div, vec3 col) {
    vec2 coord = pos.xz * scale;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1);
    float minimumx = min(derivative.x, 1);

    vec4 color = vec4(col, 1 - min(line, 1.0));
    if (scale >= 10.0) {
    // z axis
    if(pos.x > -0.2 * minimumx && pos.x < 0.2 * minimumx ) {
        color.rg = vec2(0);
        color.b = (0.3);
    }
    // x axis
    if(pos.z > -0.2 * minimumz && pos.z < 0.2 * minimumz) {
        color.bg = vec2(0);
        color.r = (0.3);
    }
    }
    color.a *= div;
    return color;
}

void main()
{
    mat4 view = push.cam.view;
    mat4 proj = push.cam.proj;

    float y = -innear.y / (infar.y - innear.y); // parametric equation of a line (y axis)
    vec3 clippos = innear + y * (infar - innear);

    if (y < 0) {
        discard;
    }
    gl_FragDepth = clamp(ClipSpaceDepth(clippos, proj, view), 0, 1);

    float lineardepth = clamp(LinearDepth(clippos, proj, view), 0, 1);
    float fading = smoothstep(0, 1, lineardepth);

    out_frag_color.rgba = (grid(clippos, 0.1, 0.4, vec3(0.05, 0.05, 0.05))).rgba;
    out_frag_color.rgba += (grid(clippos, 1, 0.3, vec3(0.3, 0.3, 0.3))).rgba;
    out_frag_color.rgba += (grid(clippos, 10, 0.2, vec3(1.0))).rgba;
    out_frag_color.a *= 0.7;
    out_frag_color.a *= fading;
}
