#version 450
#include "defines/defines.h"

#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec4 incolor;
layout(location = 1) in vec2 incoord;
layout(location = 2) in vec2 insize;
layout(location = 3) flat in uint intexture_bind;
layout(location = 4) flat in uint has_outline;

layout(location = 0) out vec4 out_frag_color;

float sdf_box(in vec2 p, in vec2 b) {
    vec2 d = abs(p) - b;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
}
void main()
{
    vec4 color = vec4(1);
    if (intexture_bind > 0)
        color.a = texture(textures[nonuniformEXT(intexture_bind)], incoord.xy).r;

    if (has_outline >= 1) {
            vec2  d    = min(incoord, 1.0 - incoord) * insize;
    float edge = min(d.x, d.y);

        float stroke_w = 0.1;
    float w = fwidth(edge); // anti-alias width
    float border = 1.0 - smoothstep(stroke_w - w, stroke_w + w, edge);
        // float dist = sdf_box(incoord.xy * 2.0 - 1.0f, vec2(1));
        // float stroke_w = 0.01;
        // float s = 1.0f - smoothstep(0, 0.05, abs(dist) - stroke_w);
        color.a = border;
    }
    out_frag_color = color.xyzw;
}
