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

layout(std430, buffer_reference, buffer_reference_align = 64) buffer InstanceData
{
    mat4 model;
};
layout(std430, buffer_reference, buffer_reference_align = 64) buffer Instance
{
    InstanceData data[];
};

layout( push_constant ) uniform constants
{
    // uint gpu_address;
    Camera cam;
    Instance inst;
    uint texture_id;
} push;

