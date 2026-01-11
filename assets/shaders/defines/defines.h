#extension GL_EXT_nonuniform_qualifier : enable

#define BindlessDescriptorSet 0

#define BindlessUniformBinding 0
#define BindlessStorageBinding 1
#define BindlessSamplerBinding 2
#define BindlessComputeBinding 3

#define GetLayoutVariableName(Name) u##Name##Register

#define RegisterUniform(Name, Struct) \
    layout(set = BindlessDescriptorSet, binding = BindlessUniformBinding) \
        uniform Name Struct \
        GetLayoutVariableName(Name)[]

#define RegisterBuffer(Layout, BufferAccess, Name, Struct) \
    layout(Layout, set = BindlessDescriptorSet, binding = BindlessStorageBinding) \
    BufferAccess buffer Name Struct GetLayoutVariableName(Name)[]

#define RegisterBufferReadWrite(Layout, Name, Struct) \
    layout(Layout, set = BindlessDescriptorSet, binding = BindlessComputeBinding) \
    buffer Name Struct GetLayoutVariableName(Name)[]

#define RegisterBufferCompute(Layout, BufferAccess, Name, Struct) \
    layout(Layout, set = BindlessDescriptorSet, binding = BindlessComputeBinding) \
    BufferAccess buffer Name Struct GetLayoutVariableName(Name)[]


#define GetResource(Name, Index) \
    GetLayoutVariableName(Name)[Index]

RegisterUniform(DummyUniform, {uint ignore; });
RegisterBuffer(std430, readonly, DummyBuffer, { uint ignore; });
RegisterBufferCompute(std430, readonly, DummyBufferCompute, { uint ignore; });
RegisterBufferReadWrite(std430, DummyStorage, { uint ignore; });

layout(set = BindlessDescriptorSet, binding = BindlessSamplerBinding) \
    uniform sampler2D textures[];
layout(set = BindlessDescriptorSet, binding = BindlessSamplerBinding) \
    uniform samplerCube cubemaps[];

layout( push_constant ) uniform constants
{
    int bindtexture;
    int bindstorage;
    int bindinstance;
    int bindbuffer;

    int vertex_out_offset;
    int vertex_in_offset;
    int inst_index;
    int pad2;

} pushconstants;

int GetStorageInd() {
  return pushconstants.bindstorage;
}
int GetInstanceInd() {
  return pushconstants.bindinstance;
}
int GetTextureInd() {
  return pushconstants.bindtexture;
}
int GetUniformInd() {
  return pushconstants.bindbuffer;
}

#define MAX_POINT_LIGHT 512 
RegisterUniform(Camera, {
    vec4 viewpos;
    vec4 mousepos;
    vec4 viewport;
    vec4 global_light_dir;
    vec4 diffuse;
    vec4 specular;
    vec4 ambient;
    vec4 point_light_pos[MAX_POINT_LIGHT];
    vec4 point_light_color[MAX_POINT_LIGHT];
    vec4 point_light_radius[MAX_POINT_LIGHT];
    
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 skybox_proj;
    mat4 shadow_matrix;
    mat4 global_transform;
    
    float time;          
    int point_light_size;
    int cubemapbind;
    int pad0;

    bool hasshadows;
    int compute_skinning_size;
    bool hascubemap;   
    int global_animation_bind;
});

#define DEPTH_ARRAY_SCALE 1000 
#define MAX_INSTANCES 10500 
const int MAX_BONES = 200;

#define NUM_PARTICLES_PER_WORKGROUP 64
#define NUM_PARTICLES (32 * 1024)

#define GIZMO_X 0
#define GIZMO_Y 1
#define GIZMO_Z 2

struct InstanceData {

    mat4 bonesmatrices[MAX_BONES];
    mat4 anim_transforms[MAX_BONES];
    mat4 rendermatrix;
    mat4 rotation;
    mat4 scale;

    vec4 position;
    vec4 gizmo_dist;
    
    uint hasanimation;
    uint texturebind;
    uint storagebind;
    uint bufferbind;
    uint objectID;
    uint selected;
    uint boneID;
    uint gizmo;
    uint anim_ind;
    uint pad0;
    uint pad1;
    uint pad2;

};
#define MEMCPY_TEST
struct AnimMatricies {
    // mat4 bonesmatrices[MAX_BONES];
    mat4 nodeOffset[118];
    mat4 nodeTransform[118];

    // mat4 rot_out[118];
    mat4 rot_start[118];
    mat4 rot_end[118];

    // vec4 pos_out[118];
    vec4 pos_start[118];
    vec4 pos_end[118];

    // vec4 scale_out[118];
    vec4 scale_start[118];
    vec4 scale_end[118];
};

struct AnimFloats {
    float rot_factor[118];
    float pos_factor[118];
    float scale_factor[118];

    // int rot_comp[118];
    // int pos_comp[118];
    // int scale_comp[118]; 
    // int animisempty[118];
    // int nodesettransform[118];
    int nodeIndex[118];
    int nodeAnimInd[118];
    int nodeBoneInd[118];
};
struct AnimationComputeData {
#ifndef MEMCPY_TEST
    mat4 nodeOffset[118];
    mat4 nodeTransform[118];

    // mat4 rot_out[118];
    mat4 rot_start[118];
    mat4 rot_end[118];

    // vec4 pos_out[118];
    vec4 pos_start[118];
    vec4 pos_end[118];

    // vec4 scale_out[118];
    vec4 scale_start[118];
    vec4 scale_end[118];

    
    float rot_factor[118];
    float pos_factor[118];
    float scale_factor[118];

    // int rot_comp[118];
    // int pos_comp[118];
    // int scale_comp[118]; 
    // int animisempty[118];
    int nodesettransform[118];
    int nodeIndex[118];
    int nodeAnimInd[118];
    int nodeBoneInd[118];

#else
    AnimMatricies matricies;
    
    AnimFloats floats;
#endif

    int animsize;
    int rootssize;
    float timetick;
    int instance_buffer_ind;

    // NodeAnimCompute animnodes[MAX_BONES];
        // NodeRootsCompute noderoots[MAX_BONES];
        // //
        // mat4 global_transform;
        // int animsize;
        // int rootssize;
        // float timetick;
        // float pad0;
    };

RegisterBufferReadWrite(std140, Instance, {
      InstanceData instances[];
});

RegisterBufferReadWrite(std430,  Animation, {
       AnimationComputeData animations[];
});

struct VertexInputData {
    vec4 vposition;
    vec4 vnormal;
    vec4 vcolor;
    vec4 vuv;
    vec4 vweight;
    ivec4 vboneid;
    
    ivec4 datas;

};
struct VertexOutputHelper {
        vec4 data;
    };

struct VertexOutputData {
        vec4 vposition;
        vec4 vnormal;
        vec4 vcolor;
        vec4 vuv;
        // glm::vec4 vweight;
        // glm::ivec4 vboneid;
   
    // vec4 datas;
        // int offset;
        // int vertex_count;
        // int pad0;
        // int pad1;
};


RegisterBufferCompute(std430, readonly, Skinning_In, {
    VertexInputData vertecies[];
});

RegisterBufferCompute(std430, readonly, Skinning_Helper, {
    VertexOutputHelper data[];
});
RegisterBufferCompute(std430, writeonly, Skinning_Out, {
    VertexOutputData vertecies_out[];
});

RegisterBuffer(std430, writeonly, Storage, {
    uint data[DEPTH_ARRAY_SCALE];
});

RegisterBufferReadWrite(std430, Compute, {
    vec2 position[NUM_PARTICLES];
    vec2 velocity[NUM_PARTICLES];
    vec4 color[NUM_PARTICLES];
});


