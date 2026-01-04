#version 450

// #include "defines/vertdef.h"
layout (location = 0) in vec4 vposition;
layout (location = 1) in vec4 vnormal;
layout (location = 2) in vec4 vcolor;
layout (location = 3) in vec4 vuv;
layout (location = 4) in vec4 vweight;
layout (location = 5) in ivec4 vboneid;
layout (location = 6) in ivec4 datas;

layout (location = 0) out vec4 outpos;
layout (location = 1) out vec4 outnormal;
layout (location = 2) out vec4 outcolor;
layout (location = 3) out vec4 outcoord;
layout (location = 4) out vec4 outweight;
layout (location = 5) out ivec4 outboneid;
layout (location = 6) out ivec4 outdatas;

layout(location = 7) out uint texturebind;
layout(location = 8) out uint storagebind;
layout(location = 9) out uint bufferbind;
layout(location = 10) out uint objectID;
layout(location = 11) out uint selected;
layout(location = 12) out uint boneID;
layout(location = 13) out uint gizmo;
out gl_PerVertex {
    vec4 gl_Position;
};
#include "defines/defines.h"

#include "defines/instance.h"


void main()
{
    
    DefineInstanceResources();
    mat4 view = mat4(mat3(GetResource(Camera, bufferbind).view ));
    gl_Position =GetResource(Camera, bufferbind).skybox_proj * view * vec4(vposition.xyz, 1.0f);  
    outcolor = vcolor;
    outcoord = vuv;
    outpos = vec4(vposition.xyz, 1.0f);
}
