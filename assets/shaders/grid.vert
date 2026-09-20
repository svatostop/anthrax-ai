#version 450

#include "defines/defines.h"
#include "defines/helpers.h"
// #include "defines/vert_def.h"

layout (location = 0) out vec3 outnear;
layout (location = 1) out vec3 outfar;

out gl_PerVertex {
    vec4 gl_Position;
};

vec3 gridplane[6] = vec3[](
    vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);

void main() {
    vec3 pos = gridplane[gl_VertexIndex].xyz;

    mat4 view = push.cam.view;
    mat4 proj = push.cam.proj;

    outnear = UnprojectPos(vec3(pos.x, pos.y, 1.0), view, proj);
    outfar = UnprojectPos(vec3(pos.x, pos.y, 0.00001), view, proj);
    gl_Position = vec4(pos, 1.0);
}
