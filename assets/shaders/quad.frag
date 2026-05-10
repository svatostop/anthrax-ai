#version 450
// #include "defines/frag_def.h"
#include "defines/defines.h"

layout (location = 0) in vec2 incoord;
layout (location = 0) out vec4 out_frag_color;

void main()
{
    vec4 color = texture(textures[push.texture_id], incoord.xy);
    out_frag_color = vec4(color.xyz, 1);//vec4(1 / gl_FragCoord.x, 1 / gl_FragCoord.y ,0,1);
}
