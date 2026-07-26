#version 460
#include "defines/defines.h"
#include "defines/vert_def.h"
layout (location = 4) in vec4 vweights;
layout (location = 5) in ivec4 vbones;

// #extension GL_EXT_debug_printf : enable
void main()
{
    mat4 view = push.cam.view;
    mat4 proj = push.cam.proj;
    mat4 model = push.inst.data[gl_InstanceIndex].model;
    mat4 skin_transform = mat4(1);
    for (int i = 0; i < 4; i++) {
        // if (vbones[i] <= 0)
            // continue;
        skin_transform += vweights[i] * push.skin.transforms[vbones[i]]; 
    }
    // debugPrintfEXT("instanceIndex=%d | %f,%f,%f\n", gl_InstanceIndex, model[3][0], model[3][1], model[3][2]);
    vec4 position = proj * view * model * skin_transform * vec4(vposition.xyz, 1.0f);
    gl_Position = position;

    outnormal = vnormal;
}
