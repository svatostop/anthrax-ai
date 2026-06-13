#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference : require

#define BindlessDescriptorSet 0

#define BindlessSamplerBinding 0

layout(set = BindlessDescriptorSet, binding = BindlessSamplerBinding) \
    uniform sampler2D textures[];

layout(std430, buffer_reference, buffer_reference_align = 64) buffer Camera
{
    vec4 viewport; 
    vec4 test_color;

    mat4 view;
    mat4 proj;
};

layout( push_constant ) uniform constants
{
    // uint gpu_address;
    Camera cam;
    uint texture_id;
} push;

// int GetStorageInd() {
//   return pushconstants.bindstorage;
// }
// int GetInstanceInd() {
//   return pushconstants.bindinstance;
// }
// int GetTextureInd() {
//   return pushconstants.bindtexture;
// }
// int GetUniformInd() {
//   return pushconstants.bindbuffer;
// }

// #define MAX_POINT_LIGHT 512 
// RegisterUniform(Camera, {
//     vec4 viewpos;
//     vec4 mousepos;
//     vec4 viewport;
//     vec4 global_light_dir;
//     vec4 diffuse;
//     vec4 specular;
//     vec4 ambient;
//     vec4 point_light_pos[MAX_POINT_LIGHT];
//     vec4 point_light_color[MAX_POINT_LIGHT];
//     vec4 point_light_radius[MAX_POINT_LIGHT];
//
//     mat4 model;
//     mat4 view;
//     mat4 proj;
//     mat4 skybox_proj;
//     mat4 shadow_matrix;
//     mat4 global_transform;
//
//     float time;          
//     int point_light_size;
//     int cubemapbind;
//     int pad0;
//
//     bool hasshadows;
//     int compute_skinning_size;
//     bool hascubemap;   
//     int global_animation_bind;
// });

