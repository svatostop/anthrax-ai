#version 460
#include "defines/vert_def.h"
#include "defines/defines.h"

void main()
{
    mat4 view = push.cam.view;
    mat4 proj = push.cam.proj;
    mat4 model = push.inst.data[gl_InstanceIndex].model;
    vec4 position = proj * view * vec4(vposition.xyz, 1.0f);
    gl_Position = position;
}
