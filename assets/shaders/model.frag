#version 450

#include "defines/fragdef.h"
#include "defines/defines.h"


void main()
{
    vec4 mousepos = GetResource(Camera, bufferbind).mousepos;

    uint depth_ind = uint(gl_FragCoord.z * DEPTH_ARRAY_SCALE);
    if( length(mousepos.xy - gl_FragCoord.xy) < 2)
    {
        GetResource(Storage, storagebind).data[depth_ind] = objectID;
    }

vec2 uv = incoord.xy;
//uv.y *= -1;
    vec4 color = texture(textures[texturebind], uv.xy).xyzw;
    if (selected == 1) {
        color.rgb += vec3(0.1);
        color.rgb = clamp(color.rgb, 0.0, 1.0);
    }
    // if (boneID != -1) {
    //     for (int i = 0; i < 4; i++) {
    //         if (boneID == inboneid[i]) {
    //             if (inweight[i] >= 0.7) {
    //                 color.xyz = vec3(1, 0, 0) * inweight[i];
    //             }
    //             else if (inweight[i] >= 0.4 && inweight[i] <= 0.6) {
    //                 color.xyz = vec3(0, 1, 0) * inweight[i];
    //             }
    //             else if (inweight[i] >= 0.1) {
    //                 color.xyz = vec3(0, 0, 1) * inweight[i];
    //             }
    //         }
    //     }
    // }
    outfragcolor = color;
}
