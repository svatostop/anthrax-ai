#version 460

// #include "defines/vertdef.h"
//#extension GL_EXT_debug_printf : enable
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

    mat4 rendermatrix = GetResource(Camera, bufferbind).proj * GetResource(Camera, bufferbind).view * (GetResource(Instance, GetInstanceInd()).instances[gl_BaseInstance].rendermatrix );

    vec4 position = vec4(vposition.xyz, 1.0f) ;//* vec4(0.08,0.08,0.08,1.0);

    gl_Position = rendermatrix * position ;


    //debugPrintfEXT("instanceIndex=%d\n", gl_BaseInstance);
    outnormal = vnormal;
    outcoord = vuv;
    // outweight = vweight;
    // outboneid = vboneid;
    outpos = vec4(vposition.xyz, 1.0f);
}
