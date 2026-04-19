#version 450
#include "defines/vert_def.h"

vec3 gridplane[6] = vec3[](
    vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);

void main() {
    vec3 pos = gridplane[gl_VertexIndex].xyz;
    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
    outpos = gl_Position ;
}
