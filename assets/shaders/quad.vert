#version 450
// #include "defines/vert_def.h"

layout (location = 0) out vec2 outcoord;

vec3 gridplane[6] = vec3[](
    vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);

vec2 uvs[6] = vec2[] (
    vec2(1,1), vec2(0,0), vec2(0,1), vec2(0,0), vec2(1,1), vec2(1,0)
);

void main() {
    vec3 pos = gridplane[gl_VertexIndex].xyz;
    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
    outcoord = uvs[gl_VertexIndex];
    // outpos = gl_Position ;
}
