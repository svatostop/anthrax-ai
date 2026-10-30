#version 450

layout (location = 0) in vec2 incoord;
layout (location = 0) out vec4 out_frag_color;

void main()
{
    vec4 color = vec4(1);
    out_frag_color = vec4(color.xyz, 1);
}
