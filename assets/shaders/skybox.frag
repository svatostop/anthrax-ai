#version 450

#include "defines/defines.h"
// #include "defines/fragdef.h"
layout (location = 0) in vec4 inpos;
layout (location = 1) in vec4 innormal;
layout (location = 2) in vec4 incolor;
layout (location = 3) in vec4 incoord;
layout (location = 4) in vec4 vweight;
layout (location = 5)flat in ivec4 vboneid;
layout (location = 6)flat in ivec4 datas;

layout(location = 7)flat in uint texturebind;
layout(location = 8)flat in uint storagebind;
layout(location = 9)flat in uint bufferbind;
layout(location = 10)flat in uint objectID;
layout(location = 11)flat in uint selected;
layout(location = 12)flat in uint boneID;
layout(location = 13)flat in uint gizmo;


layout (location = 0) out vec4 outfragcolor;

void main()
{
    int cube_ind = GetResource(Camera, bufferbind).cubemapbind;
    outfragcolor = texture(cubemaps[cube_ind], vec3(inpos.xyz)).xyzw;

}
