#version 450
// #extension GL_EXT_debug_printf : enable

#include "defines/defines.h"
layout (location = 0) out vec2 outcoord;

void main() {
    gl_PointSize = 5.0;
    uint element_index = uint(gl_VertexIndex);
 // debugPrintfEXT("instanceIndex=%d | %d, %d\n", gl_VertexIndex, element_index, element_end_point);
    vec3 position = push.debug.data[element_index].p.xyz;
    mat4 view = push.cam.view;
    mat4 proj = push.cam.proj;
    mat4 model = push.debug.data[element_index].model;

    gl_Position = proj * view * model * vec4(position, 1.0);
    outcoord = vec2(1);
}
