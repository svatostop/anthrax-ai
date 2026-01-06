#pragma once

#include "anthraxAI/gamemodules/modules.h"
#include "anthraxAI/gfx/vkbase.h"
#include "anthraxAI/gfx/vkrendertarget.h"
#include "anthraxAI/utils/defines.h"
#include "anthraxAI/gfx/vkdefines.h"
#include "anthraxAI/core/windowmanager.h"
#include "anthraxAI/gfx/bufferhelper.h"
#include "anthraxAI/gfx/vkdescriptors.h"
#include "anthraxAI/gfx/vkcmdhelper.h"
#include "anthraxAI/gfx/renderhelpers.h"
#include "anthraxAI/gfx/vkpipeline.h"
#include "anthraxAI/gfx/vkmesh.h"
#include "glm/fwd.hpp"
#include <cstddef>
#include <cstdint>
#include <vulkan/vulkan_core.h>

namespace Gfx
{
    class Renderer : public Utils::Singleton<Renderer>
    {
        public:
            void CleanTextures();
                
            void CreateCommands();
            void CreateRenderTargets();
            void DestroyRenderTarget(RenderTarget* rt);
            void CreateImGuiDescSet();

            void CleanResources();

            void CopyImage(Gfx::RenderTargetsList src_id, Gfx::RenderTargetsList dst_id);

            void CreateTextures();
            void BindTextures();
            bool CreateTextureFromInfo(const std::string& texturename);
            RenderTarget CreateTexture(const std::string& path);
            bool CreateCubemaps(const std::string& name, const std::vector<std::string>& path);
            void CreateSampler(RenderTarget& rt);
            void CreateSampler(RenderTarget* rt);

            TexturesMap GetTextureMap() const { return Textures; }
            CubemapsMap GetCubemapMap() const { return Cubemaps; }
            TexturesMap GetCubemapImguiMap() const { return CubemapsImgui; }
            RenderTarget* GetTexture(const std::string& path);
            RenderTarget* GetCubemap(const std::string& path);
            RenderTarget* GetCubemapImgui(const std::string& path);

            RenderTarget* GetRT(Gfx::RenderTargetsList id) const { return RTs[id]; }
            std::vector<std::string> GetRTList();
    #ifdef COMPUTE_SKINNING
            void FillSkinningBuffer();
    #endif
            void PrepareCameraBuffer(Keeper::Camera& camera);
            void PrepareInstanceBuffer();
            void GetTransforms(InstanceData* datas, Gfx::RenderObject obj, int i);
            void PrepareStorageBuffer();
            void PrepareCompute();        
        
            void BarrierVertexCompute(int b);
            void Submit(std::function<void(VkCommandBuffer cmd)>&& function);

            void RenderUI();
            void TransferLayoutsDebug();
    
            void set_global_animation_buffer(int i, int frame )  { global_animation_bind[frame] = i;}

            int GetFrameInd() { return FrameIndex; }
            FrameData& GetFrame() { return Frames[FrameIndex]; }
            FrameData& GetFrame(uint32_t ind) { return Frames[ind]; }
            void Sync();
            uint32_t SyncFrame();
            void SetFrameInd() { FrameIndex = (FrameIndex + 1) % MAX_FRAMES; }
            long long Time;
            bool BeginFrame();
            void EndFrame();
            void EndRender();
            void StartRender(Gfx::InputAttachments inputs, AttachmentRules rules, bool multithreaded = false);
            
            void ComputeParticles(Gfx::RenderObject& object);
            void Compute(Gfx::RenderObject& object, uint32_t work_groups, uint32_t work_groups2 = 1);
            void Draw(Gfx::RenderObject& object);
            void DrawThreaded(VkCommandBuffer cmd, Gfx::RenderObject& object, Material* mat,  Gfx::MeshInfo* mesh, Gfx::MeshPushConstants& constatns, bool ismodel, uint32_t inst_ind);
            void DrawMeshes(Gfx::RenderObject& object);
            void DrawMesh(Gfx::RenderObject& object, Gfx::MeshInfo* mesh, bool ismodel);
            void DrawSimple(Gfx::RenderObject& object);
            void NullTmpBindings();
            void CheckTmpBindings(Gfx::MeshInfo* mesh, Gfx::Material* material, bool* bindpipe, bool* bindindex);

        	FrameArray Frames;
            
            void ResetSkinningIterator() { skinning_iterator = 0; }
        
            void BeginRendering(VkCommandBuffer cmd, const VkRenderingInfoKHR* renderinfo) { vkCmdBeginRenderingKHR(cmd, renderinfo); }
            void EndRendering(VkCommandBuffer cmd) { vkCmdEndRenderingKHR(cmd); }
            bool IsOnResize() const { return OnResize; }
            void SetOnResize(bool ison) { OnResize = ison; }

            int GetInstanceSize() const { return 20; }

            const glm::vec3 GetCameraPos() const { return glm::vec3(CamData.viewpos); }
            const glm::mat4& GetProjection() const { return CamData.proj; }
            const glm::mat4& GetView() const { return CamData.view; }

            void ResetInstanceInd() { InstanceIndex = 0; }
            void IncInstanceInd(int size) { InstanceIndex += size; }
            uint32_t GetInstanceInd() { return InstanceIndex; }// += size; }
            void SetInstanceInd(uint32_t i) { InstanceIndex = i; }// += size; }

            void SetUpdateSamplers(bool upd) { UpdateSamples = upd; }
            bool GetUpdateSamplers() const { return UpdateSamples; }

            void DebugRenderName(const std::string& str);
            void EndRenderName();

            VkCommandBuffer GetCmd() { return Cmd.GetCmd(); }
            Gfx::CommandBuffer& GetCmdHandle() { return Cmd; }
            void SetCmd(VkCommandBuffer cmd) { Cmd.SetCmd(cmd); }

            void InitTracy();
            void DestroyTracy();

            TracyVkCtx GetTracyContext() { return Tracy.Context[GetFrameInd()]; }
            VkCommandBuffer GetTracyCmd() { return Tracy.Cmd; }

            void SetGlobalLightDir(glm::vec3 dir) { LightData.GlobalDirection = dir; }
            glm::vec3 GetGlobalLightDir() { return LightData.GlobalDirection; }
            void SetAmbient(glm::vec3 a) { LightData.Ambient = a; }
            glm::vec3 GetAmbient() { return LightData.Ambient; }
            void SetSpecular(glm::vec3 a) { LightData.Specular = a; }
            glm::vec3 GetSpecular() { return LightData.Specular; }
            void SetDiffuse(glm::vec3 a) { LightData.Diffuse = a; }
            glm::vec3 GetDiffuse() { return LightData.Diffuse; }

            void InitDrawIndirect();
            void CompactDrawIndirect(const Modules::RenderQueueMap& map);
            void CompactIndirect(Material* mat, MeshInfo* mesh, int i, bool is_instanced);
            void ClearIndirectBatches() { indirect_batch.clear(); }
            // void RenderIndirect(const Modules::RenderQueueMap& map);
            void RenderIndirect( Modules::Module& module);
            void RenderIndirectCall(IndirectBatch& batch, MeshInfo* mesh, RenderObject& testobj);

            bool GetCubemapRendering() { return HasFrameCubemap; }
            void SetCubemapRendering(bool cube) { HasFrameCubemap = cube; }


            int ComputeSkinningMaxVertex = 0;
        private:
            BufferHelper::Buffer DrawIndirect;
            std::vector<Gfx::IndirectBatch> indirect_batch;
            
            RenderTarget* RTs[RT_SIZE];
            TexturesMap Textures;
            CubemapsMap Cubemaps;
            TexturesMap CubemapsImgui;
            
            StorageData StorageBuffer;
            InstanceData InstanceBuffer;
            AnimationComputeData AnimationBuffer;
            CameraData 	CamData;
            ComputeData CompData;
            LightsData LightData;
            UploadContext Upload;

            TracyInfo Tracy;

            uint32_t InstanceCount = 0;
            uint32_t InstanceIndex = 0;
            
            std::unordered_map<int, int> united_vertex_offsets;
            std::unordered_map<int, int> final_mapped_united_vertex_offsets;
            std::vector<int> final_vertex_offsets;
            int skinning_iterator = 0;
            bool HasFrameCubemap = false;
            bool UpdateSamples = false;
            bool OnResize = false;
	        int FrameIndex = 0;
            uint32_t SwapchainIndex = 0;
            uint32_t PrevSwapchainIndex = 0;
            uint32_t global_animation_bind[MAX_FRAMES] ={ 0, 0, 0};

            VkRenderingAttachmentInfoKHR AttachmentInfos[Gfx::RT_SIZE];
            PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR{VK_NULL_HANDLE};
        	PFN_vkCmdEndRenderingKHR   vkCmdEndRenderingKHR{VK_NULL_HANDLE};

            Gfx::CommandBuffer Cmd;

            Gfx::Material* TmpBindMaterial = nullptr;
	        Gfx::MeshInfo* TmpBindMesh = nullptr;
    std::vector<VkTimeDomainEXT>time_domains;
    std::vector<VkCalibratedTimestampInfoEXT>       timestamps_info{};        // This vector is essential to vkGetCalibratedTimestampsEXT, and only need to be registered once.
    std::vector<uint64_t>        timestamps{};                       // timestamps vector
	std::vector<uint64_t>        max_deviations{};  

    };
}
