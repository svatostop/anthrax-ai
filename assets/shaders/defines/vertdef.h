// layout (location = 0) in vec4 vposition;
// layout (location = 1) in vec3 vnormal;
// layout (location = 2) in vec3 vcolor;
// layout (location = 3) in vec2 vuv;
// layout (location = 4) in vec4 vweight;
// layout (location = 5) in ivec4 vboneid;
// #ifndef SKINNING_IN_DECL
layout (location = 0) in vec4 vposition;
layout (location = 1) in vec4 vnormal;
layout (location = 2) in vec4 vcolor;
layout (location = 3) in vec4 vuv;

layout (location = 0) out vec4 outpos;
layout (location = 1) out vec4 outnormal;
layout (location = 2) out vec4 outcolor;
layout (location = 3) out vec4 outcoord;

layout(location = 4) out uint texturebind;
layout(location = 5) out uint storagebind;
layout(location = 6) out uint bufferbind;
layout(location = 7) out uint objectID;
layout(location = 8) out uint selected;
layout(location = 9) out uint boneID;
layout(location = 10) out uint gizmo;

// #else
// #endif
// layout (location = 0) out vec4 outpos;
// layout (location = 1) out vec3 outnormal;
// layout (location = 2) out vec3 outcolor;
// layout (location = 3) out vec2 outcoord;
// layout (location = 4) out vec4 outweight;
// layout (location = 5) out ivec4 outboneid;
// layout(location = 6) out uint texturebind;
// layout(location = 7) out uint storagebind;
// layout(location = 8) out uint bufferbind;
// layout(location = 9) out uint objectID;
// layout(location = 10) out uint selected;
// layout(location = 11) out uint boneID;
// layout(location = 12) out uint gizmo;
//
out gl_PerVertex {
    vec4 gl_Position;
};


