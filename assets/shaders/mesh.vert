#version 460
#include "defines/vert_def.h"
#include "defines/defines.h"

// #extension GL_EXT_debug_printf : enable
void main()
{
    mat4 view = push.cam.view;
    mat4 proj = push.cam.proj;
    mat4 model = push.inst.data[gl_InstanceIndex].model;
    // debugPrintfEXT("instanceIndex=%d | %f,%f,%f\n", gl_InstanceIndex, model[3][0], model[3][1], model[3][2]);
    vec4 position = proj * view * model * vec4(vposition.xyz, 1.0f);
    gl_Position = position;
    outnormal = vec4(vnormal, 1);
}
