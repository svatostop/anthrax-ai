#version 450
// #extension GL_EXT_debug_printf : enable

#include "defines/defines.h"
layout (location = 0) out vec2 outcoord;

void main() {

    uint element_index =  gl_InstanceIndex * 2;
 // debugPrintfEXT("instanceIndex=%d | %d, %d\n", gl_VertexIndex, element_index, element_end_point);
    vec3 position = gl_VertexIndex == 0u ? push.debug.data[element_index].p.xyz : push.debug.data[element_index + 1].p.xyz;
    mat4 view = push.cam.view;
    mat4 proj = push.cam.proj;
    mat4 model = push.debug.data[element_index].model;

    gl_Position = proj * view * model * vec4(position, 1.0);
    outcoord = vec2(1);
}
