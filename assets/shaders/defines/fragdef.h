// layout (location = 0) in vec4 inpos;
// layout (location = 1) in vec3 innormal;
// layout (location = 2) in vec3 incolor;
// layout (location = 3) in vec2 incoord;
// layout (location = 4) in vec4 inweight;
// layout (location = 5) flat in ivec4 inboneid;
//

// #ifndef SKINNING_IN_DECL
layout (location = 0) in vec4 inpos;
layout (location = 1) in vec4 innormal;
layout (location = 2) in vec4 incolor;
layout (location = 3) in vec4 incoord;

layout(location = 4)flat in uint texturebind;
layout(location = 5)flat in uint storagebind;
layout(location = 6)flat in uint bufferbind;
layout(location = 7)flat in uint objectID;
layout(location = 8)flat in uint selected;
layout(location = 9)flat in uint boneID;
layout(location = 10)flat in uint gizmo;
// #else
// #endif
// layout(location = 6)flat in uint texturebind;
// layout(location = 7)flat in uint storagebind;
// layout(location = 8)flat in uint bufferbind;
// layout(location = 9)flat in uint objectID;
// layout(location = 10)flat in uint selected;
// layout(location = 11)flat in uint boneID;
// layout(location = 12)flat in uint gizmo;
//

layout (location = 0) out vec4 outfragcolor;

//layout(set = 1, binding = 0) uniform sampler2D texturesampler;

#define GetTextureIndi() texturebind

