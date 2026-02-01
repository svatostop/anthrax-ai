#version 460

#include "defines/vertdef.h"
layout(location = 11) out uint ireflection;
#include "defines/defines.h"
#include "defines/instance.h"
//#extension GL_EXT_debug_printf : enable

 void main()
{
    DefineInstanceResources();
    ireflection = GetResource(Instance, GetInstanceInd()).instances[gl_InstanceIndex].reflection;
    uint hasanim = GetResource(Instance, GetInstanceInd()).instances[gl_InstanceIndex].hasanimation;
    mat4 bonetransforms = mat4(1.0f);
    // if (hasanim == 1) {
    //     bonetransforms = GetResource(Instance, GetInstanceInd()).instances[gl_InstanceIndex].bonesmatrices[vboneid[0]] * vweight[0];
    //     bonetransforms += GetResource(Instance, GetInstanceInd()).instances[gl_InstanceIndex].bonesmatrices[vboneid[1]] * vweight[1];
    //     bonetransforms += GetResource(Instance, GetInstanceInd()).instances[gl_InstanceIndex].bonesmatrices[vboneid[2]] * vweight[2];
    //     bonetransforms += GetResource(Instance, GetInstanceInd()).instances[gl_InstanceIndex].bonesmatrices[vboneid[3]] * vweight[3];
    // }

    mat4 rendermatrix = GetResource(Camera, bufferbind).proj * GetResource(Camera, bufferbind).view * GetResource(Instance, GetInstanceInd()).instances[gl_InstanceIndex].rendermatrix * GetResource(Instance, GetInstanceInd()).instances[gl_InstanceIndex].rotation* GetResource(Instance, GetInstanceInd()).instances[gl_InstanceIndex].scale;;

    vec4 position = bonetransforms * vec4(vposition.xyz, 1.0f);
    gl_Position = rendermatrix * position;
    
   // debugPrintfEXT("instanceIndex=%d | %d\n", gl_BaseInstance, GetUniformInd());
    outcoord = vuv;
    // outweight = vweight;
    // outboneid = vboneid;
    mat4 m = GetResource(Instance, GetInstanceInd()).instances[gl_InstanceIndex].rendermatrix;
    vec4 p = m * vec4(vposition.xyz, 1.0);
    outpos = p;
    outnormal = vnormal;//vec4(GetResource(Instance, GetInstanceInd()).instances[gl_InstanceIndex].rendermatrix * (vnormal)).xyzw ;// transpose(inverse(mat3(m))) * vnormal;

}
