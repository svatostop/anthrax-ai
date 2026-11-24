#version 450

#include "defines/defines.h"
layout (location = 0) in vec4 vposition;
layout (location = 1) in vec3 vnormal;
layout (location = 2) in vec3 vcolor;
layout (location = 3) in vec2 vuv;

layout(location = 6) in vec2 instance_pos;
layout(location = 7) in vec2 instance_size;
layout(location = 8) in vec4 instance_uv;
layout(location = 9) in vec4 instance_color;
layout(location = 10) in uint intexture_bind;
layout(location = 11) in uint has_outline;

layout (location = 0) out vec4 outcolor;
layout (location = 1) out vec2 outcoord;
layout (location = 2) out vec2 outsize;
layout(location = 3) flat out uint outtexture_bind;
layout(location = 4) flat out uint outhas_outline;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    vec2 fin_pos;
    fin_pos.x = instance_pos.x + vposition.x * instance_size.x;
    fin_pos.y = instance_pos.y + vposition.y * instance_size.y;
    vec4 viewport = push.cam.viewport; 
    vec2 ndc = ((fin_pos) / viewport.xy) * 2.0 - 1.0;
    gl_Position = vec4(ndc, 0, 1.0);
    outcoord = mix(instance_uv.xy, instance_uv.zw, vuv.xy);
    outcolor = vec4(1);
    outsize = instance_size.xy;
    outtexture_bind = intexture_bind;
    outhas_outline = has_outline;
}
