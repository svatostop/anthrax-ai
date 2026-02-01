#pragma once
#include "anthraxAI/gfx/vkdefines.h"
#include "anthraxAI/gfx/vkrendertarget.h"
#include "anthraxAI/gfx/model.h"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "anthraxAI/utils/tracy.h"
#include "glm/fwd.hpp"

#define MEMCPY_TEST
namespace Gfx
{
    enum BindlessDataType {
        BINDLESS_DATA_NONE = 0,
        BINDLESS_DATA_CAM_STORAGE_SAMPLER,
        BINDLESS_DATA_CAM_BUFFER,
        BINDLESS_DATA_PARTICLES,
        BINDLESS_DATA_STORAGE,
        BINDLESS_DATA_SKINNING,
        BINDLESS_DATA_SIZE
    };
    struct Material {
        VkPipelineLayout PipelineLayout;
        VkPipeline Pipeline;
    };

    struct RenderObject {
	    Gfx::MeshInfo* Mesh = nullptr;
	    Gfx::ModelInfo* Model[MAX_FRAMES] = { nullptr };
	    Gfx::Material* Material = nullptr;
        Gfx::RenderTarget* Texture = nullptr;
        std::vector<Gfx::RenderTarget*> Textures;

        std::string MaterialName;
        std::string TextureName;

	    Vector3<float> Position;
        glm::vec3 rotation;
        float scale;
        bool reflection = false;

        bool HasAnimation = false;
        bool IsCompute = false;
        bool VertexBase = false;
        bool IsGrid = false;
        bool HasStorage = false;

        bool IsVisible = true;
        uint32_t ID;
        bool IsSelected = false;

        uint32_t GizmoType = 0;
        bool Spawn = false;
        std::string SpawnName;
        uint32_t instance_size = 0;

        uint32_t SkinningHelperBind[MAX_FRAMES];
        uint32_t BufferBind[MAX_FRAMES];
        uint32_t StorageBind[MAX_FRAMES];
        uint32_t InstanceBind[MAX_FRAMES];
        uint32_t TextureBind[MAX_FRAMES];
        
        std::string ModelName;
        bool skinned_vertex = false;
        bool input_vertex = false;
        bool output_vertex = false;
        float AnimOffset = 1.0;
    };
    struct IndirectBatch{
        MeshInfo* mesh;
        Material* material;
        uint32_t first;
        uint32_t count = 1;
    };
    #define MAX_COMMANDS 5000 
    #define DEPTH_ARRAY_SCALE 1000 
    #define MAX_BONES 200
    #define MAX_INSTANCES 10500 
    #define BONE_ARRAY_SIZE (sizeof(glm::mat4) * MAX_BONES)
    
    #define NUM_PARTICLES_PER_WORKGROUP 64
    #define NUM_PARTICLES (32 * 1024)
    struct ComputeData {
        glm::vec2 position[NUM_PARTICLES];
        glm::vec2 velocity[NUM_PARTICLES];
        glm::vec4 color[NUM_PARTICLES];
    };

    struct StorageData {
        u_int data[DEPTH_ARRAY_SCALE] = {0};
    };
    struct InstanceData {
        glm::mat4 bonesmatrices[MAX_BONES];
        glm::mat4 anim_transforms[MAX_BONES];
        glm::mat4 rendermatrix;
        glm::mat4 rotation;
        glm::mat4 scale;

        glm::vec4 position;
        glm::vec4 gizmo_dist;
        
        uint32_t hasanimation = 0;
        uint32_t texturebind = 0;
        uint32_t storagebind = 0;
        uint32_t bufferbind = 0;
        uint32_t objectID = 0;
        uint32_t selected = 0;
        uint32_t boneID = 0;
        uint32_t gizmo = 0;
        uint32_t anim_ind = 0;
        uint32_t reflection;
        uint32_t pad1;
        uint32_t pad2;
    };
    struct AnimFloats {
        float rot_factor[118];
        float pos_factor[118];
        float scale_factor[118];
    
        // int rot_comp[108];
        // int pos_comp[108];
        // int scale_comp[108]; 
        // int animisempty[108];
        // int nodesettransform[108];
        int nodeIndex[118];
        int nodeAnimInd[118];
        int nodeBoneInd[118];
    };
    struct AnimMatricies {
        // glm::mat4 bonesmatrices[MAX_BONES];
        glm::mat4 nodeOffset[118];
        glm::mat4 nodeTransform[118];

        // glm::mat4 rot_out[108];
        glm::mat4 rot_start[118];
        glm::mat4 rot_end[118];

    //
       // glm::vec4 pos_out[108];
       glm::vec4 pos_start[118];
       glm::vec4 pos_end[118];

       // glm::vec4 scale_out[108];
       glm::vec4 scale_start[118];
       glm::vec4 scale_end[118];
    };
#ifndef MEMCPY_TEST
    struct AnimationComputeData {
        glm::mat4 nodeOffset[108];
        glm::mat4 nodeTransform[108];

        // glm::mat4 animrot[200];
        glm::mat4 rot_out[108];
        glm::mat4 rot_start[108];
        glm::mat4 rot_end[108];

    //
       glm::vec4 pos_out[108];
       glm::vec4 pos_start[108];
       glm::vec4 pos_end[108];

       glm::vec4 scale_out[108];
       glm::vec4 scale_start[108];
       glm::vec4 scale_end[108];
    //

       
        float rot_factor[108];
        float pos_factor[108];
        float scale_factor[108];
        
        int rot_comp[108];
        int pos_comp[108];
        int scale_comp[108]; 
       int animisempty[108];
       // int nodesettransform[108];
       int nodeIndex[108];
       int nodeAnimInd[108];
       int nodeBoneInd[108];

       int animsize = 0;
       int rootssize = 0;
       float timetick = 0;
       float pad0 = 0;
    };
#else

    struct AnimationComputeData {
        
        AnimMatricies matricies;  
            
        AnimFloats floats;

        int animsize = 0;
        int rootssize = 0;
        float timetick = 0;
        int instance_buffer_ind = 0;
    };

#endif


    #define MAX_POINT_LIGHT 512 
    struct CameraData {
        glm::vec4 viewpos;
        glm::vec4 mousepos;
        glm::vec4 viewport;
        glm::vec4 global_light_dir;
        glm::vec4 diffuse;
        glm::vec4 specular;
        glm::vec4 ambient;
        glm::vec4 point_light_pos[MAX_POINT_LIGHT];
        glm::vec4 point_light_color[MAX_POINT_LIGHT]; 
        glm::vec4 point_light_radius[MAX_POINT_LIGHT];

        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
        glm::mat4 skybox_proj;
        glm::mat4 shadow_matrix;
        glm::mat4 global_transform;

        float time;
        int point_light_size;
        int cubemapbind;
        int pad0;

        bool hasshadows;
        int compute_skinning_size;
        bool hascubemap;   
        int global_animation_bind;

    };
    
    struct VertexInputData {
        glm::vec4 vposition;
        glm::vec4 vnormal;
        glm::vec4 vcolor;
        glm::vec4 vuv;
        glm::vec4 vweight;
        glm::ivec4 vboneid;
   
        glm::ivec4 datas;
        // int offset;
        // int vertex_count;
        // int pad0;
        // int pad1;
};
    struct VertexOutputHelper {
        glm::vec4 data;
    };
    struct VertexOutputData {
        glm::vec4 vposition;
        glm::vec4 vnormal;
        glm::vec4 vcolor;
        glm::vec4 vuv;
        // glm::vec4 vweight;
        // glm::ivec4 vboneid;
   
    // glm::vec4 datas;
        // int offset;
        // int vertex_count;
        // int pad0;
        // int pad1;
};

    struct LightsData {
        glm::vec3 GlobalDirection = glm::vec3(30, 26, 50);//glm::vec3(0.5f, -1.0f, -1.0f);
        glm::vec3 Specular = glm::vec3(1.0f);
        glm::vec3 Ambient = glm::vec3(0.2f);
        glm::vec3 Diffuse = glm::vec3(1.0f);
    };
    
    struct SecondaryCmdInfo {
        VkCommandBuffer Cmd;
        VkCommandPool Pool;
    };
    struct FrameData {
        VkSemaphore PresentSemaphore, RenderSemaphore;
        VkFence RenderFence;
        VkCommandPool CommandPool;
        VkCommandBuffer MainCommandBuffer;
        std::vector<SecondaryCmdInfo> SecondaryCmd;
    };

    struct UploadContext {
        VkFence UploadFence;
        VkCommandPool CommandPool;
        VkCommandBuffer CommandBuffer;
    };

    struct TracyInfo {
        VkCommandPool Pool;
        VkCommandBuffer Cmd;
        TracyVkCtx Context[MAX_FRAMES]; 
        PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT GetPhysicalDeviceCalibrateableTimeDomainsEXT;
        PFN_vkGetCalibratedTimestampsEXT GetCalibratedTimestampsEXT;
    };

    class InputAttachments
    {
        public:
            void Add(Gfx::RenderTargetsList id, bool isdepth = false) { if (!isdepth) { Color = id; } else { Depth = id; } }
            void AddA(Gfx::RenderTargetsList id) { Albedo = id;  }
            void AddP(Gfx::RenderTargetsList id) { Position = id;  }
            void AddN(Gfx::RenderTargetsList id) { Normal = id;  }
            void AddH(Gfx::RenderTargetsList id) { GHelper = id;  }
            Gfx::RenderTargetsList GetColor() const { return Color; }
            Gfx::RenderTargetsList GetDepth() const { return Depth; }
            bool HasColor() const { return Color != Gfx::RT_SIZE; }
            bool HasAlbedo() const { return Albedo != Gfx::RT_SIZE; }
            bool HasPosition() const { return Position != Gfx::RT_SIZE; }
            bool HasNormal() const { return Normal != Gfx::RT_SIZE; }
            bool HasDepth() const { return Depth != Gfx::RT_SIZE; }
            bool IsColor(Gfx::RenderTargetsList id) { return id != Depth; }
            bool IsDepth(Gfx::RenderTargetsList id) { return id != Color; }
        private:
            Gfx::RenderTargetsList Color = Gfx::RT_SIZE;
            Gfx::RenderTargetsList Normal = Gfx::RT_SIZE;
            Gfx::RenderTargetsList Position = Gfx::RT_SIZE;
            Gfx::RenderTargetsList Albedo = Gfx::RT_SIZE;
            Gfx::RenderTargetsList GHelper  = Gfx::RT_SIZE;
            Gfx::RenderTargetsList Depth = Gfx::RT_SIZE;
    };

    enum AttachmentRules {
        ATTACHMENT_RULE_DONT_CARE = 1 << 0,
        ATTACHMENT_RULE_LOAD  = 1 << 1,
        ATTACHMENT_RULE_CLEAR = 1 << 2,
    };

    typedef std::array<FrameData, MAX_FRAMES + 1> FrameArray;
    typedef std::unordered_map<std::string, RenderTarget> TexturesMap;
    typedef std::unordered_map<std::string, RenderTarget> CubemapsMap;


    struct BasicParams {
        uint32_t camerabuffer = 0;
        uint32_t texturehandle = 0;
        uint32_t storagebuffer = 0;
        uint32_t instancebuffer = 0;
    };
    struct CamBufferParams {
        uint32_t camerabuffer = 0;
        uint32_t pad0;
        uint32_t pad1;
        uint32_t pad2;
    };

}
