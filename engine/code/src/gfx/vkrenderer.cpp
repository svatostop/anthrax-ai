#include "anthraxAI/gfx/vkrenderer.h"
#include "anthraxAI/core/animator.h"
#include "anthraxAI/core/scene.h"
#include "anthraxAI/gamemodules/modules.h"
#include "anthraxAI/gameobjects/gameobjects.h"
#include "anthraxAI/gfx/bufferhelper.h"
#include "anthraxAI/gfx/model.h"
#include "anthraxAI/gfx/renderhelpers.h"
#include "anthraxAI/gfx/vkdevice.h"
#include "anthraxAI/gfx/vkbase.h"
#include "anthraxAI/gfx/vkdescriptors.h"
#include "anthraxAI/core/windowmanager.h"
#include "anthraxAI/gameobjects/objects/camera.h"
#include "anthraxAI/gfx/vkmesh.h"
#include "anthraxAI/gfx/vkpipeline.h"
#include "anthraxAI/gfx/vkrendertarget.h"
#include "anthraxAI/utils/debug.h"
#include "anthraxAI/utils/defines.h"
#include "anthraxAI/utils/mathdefines.h"
#include "anthraxAI/utils/thread.h"
#include "anthraxAI/gfx/vkdefines.h"
#include "glm/common.hpp"
#include "glm/detail/qualifier.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <tracy/Tracy.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <random>

void Gfx::Renderer::RenderIndirectCall(IndirectBatch& batch, MeshInfo* mesh, RenderObject& testobj)
{
#ifdef TRACY
    ZoneScopedN("Renderer::RenderIndirectCall");
#endif
    bool bindpipe, bindindex = false;
     CheckTmpBindings(mesh, batch.material, &bindpipe, &bindindex);

	    if (bindpipe) {
	        vkCmdBindDescriptorSets(Cmd.GetCmd(), VK_PIPELINE_BIND_POINT_GRAPHICS, batch.material->PipelineLayout, 0, 1, Gfx::DescriptorsBase::GetInstance()->GetBindlessSet(GetFrameInd()), 0, nullptr);
	    	vkCmdBindPipeline(Cmd.GetCmd(), VK_PIPELINE_BIND_POINT_GRAPHICS, batch.material->Pipeline);
        }

	    Gfx::MeshPushConstants constants;
	    constants.texturebind = testobj.TextureBind[GetFrameInd()];
	    constants.bufferbind = testobj.BufferBind[GetFrameInd()];
        constants.storagebind = testobj.StorageBind[GetFrameInd()];
        constants.instancebind = testobj.InstanceBind[GetFrameInd()];
	    vkCmdPushConstants(Cmd.GetCmd(), batch.material->PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Gfx::MeshPushConstants), &constants);

	    if (bindindex) {
	    	VkDeviceSize offset = {0};
        #ifdef COMPUTE_SKINNING
            if (skinning_iterator == 0) {
                VkBuffer buffer = Gfx::DescriptorsBase::GetInstance()->GetBuffer(0, SKINNING_OUT);
	    	    vkCmdBindVertexBuffers(Cmd.GetCmd(), 0, 1, &buffer, &offset);
                skinning_iterator++;
            }
        #else 
	    	vkCmdBindVertexBuffers(Cmd.GetCmd(), 0, 1, &mesh->VertexBuffer.Buffer, &offset);
        #endif
	    	vkCmdBindIndexBuffer(Cmd.GetCmd(), mesh->IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT16);
	    }
	    
        VkDeviceSize indirect_offset = batch.first * sizeof(VkDrawIndexedIndirectCommand);
        uint32_t draw_stride = sizeof(VkDrawIndexedIndirectCommand);
        vkCmdDrawIndexedIndirect(Cmd.GetCmd(), DrawIndirect.Buffer, indirect_offset, batch.count, draw_stride);
        Utils::Debug::GetInstance()->DebugDrawCall();
}

// void Gfx::Renderer::RenderIndirect(const Modules::RenderQueueMap& map)
void Gfx::Renderer::RenderIndirect( Modules::Module& module)
{
#ifdef TRACY
    ZoneScopedN("Renderer::RenderIndirect");
#endif
    void* draw;
    VkDeviceSize size = MAX_COMMANDS * sizeof(VkDrawIndexedIndirectCommand);
    vkMapMemory(Gfx::Device::GetInstance()->GetDevice(), DrawIndirect.DeviceMemory, 0, size, 0, (void**)&draw);
    
    VkDrawIndexedIndirectCommand* draw_cmds = (VkDrawIndexedIndirectCommand*)draw;
    
    int i = 0;
    RenderObject testobj;
    skinning_iterator = 0;
    uint32_t instances_tmp = 0;
    // for (const auto& it : map) {
        // for (const RenderObject& obj : it.second) {
        for ( RenderObject& obj : module.GetSkinningRQ()) {
            if (i == 0) {
                testobj = obj;
            }
            if (obj.Model[0]) {
                const int meshsize = obj.Model[0]->Meshes.size();
	            for (int k = 0; k < meshsize; k++) {
                    draw_cmds[i].firstInstance = i;
                    draw_cmds[i].firstIndex = 0;
                    if (obj.Spawn) {
                        draw_cmds[i].instanceCount = obj.instance_size;
                        draw_cmds[i].firstInstance = instances_tmp;
                        instances_tmp += obj.instance_size;
                    }
                    else {
                        draw_cmds[i].instanceCount = 1;
                        draw_cmds[i].firstInstance = instances_tmp;
                        instances_tmp++;
                    }
                    draw_cmds[i].indexCount = obj.Model[0]->Meshes[k]->AIindices.size();
                    #ifdef COMPUTE_SKINNING
                    // printf("iter---%d|%d\n", skinning_iterator, final_vertex_offsets.size());
                    draw_cmds[i].vertexOffset = final_vertex_offsets[skinning_iterator];// i * obj.Model[0]->Meshes[k]->Vertices.size();
                    skinning_iterator++;
                    #endif
                    i++;
                }
            }
            else {
                draw_cmds[i].firstInstance = i;
                draw_cmds[i].firstIndex = 0;
                draw_cmds[i].instanceCount = 1;
                draw_cmds[i].indexCount = obj.Mesh->Indices.size();
                    i++;
            }
            // if (obj.Spawn) {
            //     break;
            // }
        }
    // }
    vkUnmapMemory(Gfx::Device::GetInstance()->GetDevice(),DrawIndirect.DeviceMemory);
    
    skinning_iterator = 0;
    for (IndirectBatch& batch : indirect_batch) {
            RenderIndirectCall(batch, batch.mesh, testobj); 
    }
    skinning_iterator = 0;
}

void Gfx::Renderer::InitDrawIndirect()
{
    VkDeviceSize size = MAX_COMMANDS * sizeof(VkDrawIndexedIndirectCommand);
    BufferHelper::CreateBuffer(DrawIndirect, size, static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |  VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT));
    DrawIndirect.tag = "indirect buffer";
    VkDebugUtilsObjectNameInfoEXT info;
	info.pNext = nullptr;
	info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	info.objectHandle = reinterpret_cast<uint64_t>(DrawIndirect.DeviceMemory);
	info.objectType = VK_OBJECT_TYPE_DEVICE_MEMORY;;
	info.pObjectName = "indirect buffer #1 frame";
	Gfx::Vulkan::GetInstance()->SetDebugName(info);

	Core::Deletor::GetInstance()->Push(Core::Deletor::Type::NONE, [=, this]() {
		vkDestroyBuffer(Gfx::Device::GetInstance()->GetDevice(), DrawIndirect.Buffer, nullptr);
    	vkFreeMemory(Gfx::Device::GetInstance()->GetDevice(), DrawIndirect.DeviceMemory, nullptr);
	});
}

void Gfx::Renderer::CompactIndirect(Material* mat, MeshInfo* mesh, int i, bool is_instanced)
{

            bool samemesh = indirect_batch.empty() ? false : mesh == indirect_batch.back().mesh;
            bool samemat = indirect_batch.empty() ? false : mat == indirect_batch.back().material;
            if (samemesh && samemat && !is_instanced) {

                indirect_batch.back().count++;
            }
            else {
                IndirectBatch batch;
                batch.mesh = mesh;
                batch.count = 1;
                batch.first = i;
                batch.material = mat;
                indirect_batch.push_back(batch);
            }
}

void Gfx::Renderer::CompactDrawIndirect(const Modules::RenderQueueMap& map)
{
#ifdef TRACY
    ZoneScopedN("Renderer::CompactDrawIndirect");
#endif
    int i = 0;
    Modules::Module& module = Core::Scene::GetInstance()->GetModule("gbuffer");
    // for (const auto& it : map) {
        // for (const RenderObject& obj : it.second) {
        for ( RenderObject& obj : module.GetSkinningRQ()) {
            if (obj.Model[0]) {
                for (MeshInfo* info : obj.Model[0]->Meshes) {
                    CompactIndirect(obj.Material, info, i, obj.Spawn);
                    i++;
                }
            }
            else {
                CompactIndirect(obj.Material, obj.Mesh, i, false);
                i++;
            }
        }
    // }
}


void Gfx::Renderer::DrawSimple(Gfx::RenderObject& object)
{
    bool bindpipe, bindindex = false;
	CheckTmpBindings(object.Mesh, object.Material, &bindpipe, &bindindex);

    if (bindpipe) {
        vkCmdBindDescriptorSets(Cmd.GetCmd(), VK_PIPELINE_BIND_POINT_GRAPHICS, object.Material->PipelineLayout, 0, 1, Gfx::DescriptorsBase::GetInstance()->GetBindlessSet(GetFrameInd()), 0, nullptr);

		vkCmdBindPipeline(Cmd.GetCmd(), VK_PIPELINE_BIND_POINT_GRAPHICS, object.Material->Pipeline);
    }

	Gfx::MeshPushConstants constants;
	constants.texturebind = object.TextureBind[GetFrameInd()];
	constants.bufferbind = object.BufferBind[GetFrameInd()];
    if (object.HasStorage) {
        constants.storagebind = object.StorageBind[GetFrameInd()];
        constants.instancebind = object.InstanceBind[GetFrameInd()];
    }
	vkCmdPushConstants(Cmd.GetCmd(), object.Material->PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Gfx::MeshPushConstants), &constants);
    if (object.IsCompute) {
        VkDeviceSize offset = {0};
        VkBuffer buffer = Gfx::DescriptorsBase::GetInstance()->GetComputeBuffer(GetFrameInd());
        vkCmdBindVertexBuffers(Cmd.GetCmd(), 0, 1, &buffer, &offset);
	    vkCmdDraw(Cmd.GetCmd(), u_int32_t(NUM_PARTICLES / (sizeof(glm::vec2) + sizeof(glm::vec2) + sizeof(glm::vec4))), 1, 0, 0);
    }
    else {
	    vkCmdDraw(Cmd.GetCmd(), 6, 1, 0, 0);
    }
    Utils::Debug::GetInstance()->DebugDrawCall();
}

void Gfx::Renderer::DrawMeshes(Gfx::RenderObject& object)
{
	const int meshsize = object.Model[GetFrameInd()]->Meshes.size();
	for (int i = 0; i < meshsize; i++) {

		DrawMesh(object, object.Model[GetFrameInd()]->Meshes[i], true);
	}
}
void Gfx::Renderer::DrawMesh(Gfx::RenderObject& object, Gfx::MeshInfo* mesh, bool ismodel)
{
#ifdef TRACY
    ZoneScopedN("Renderer::DrawMesh");
#endif
	bool bindpipe, bindindex = false;
	CheckTmpBindings(mesh, object.Material, &bindpipe, &bindindex);

	if (bindpipe) {
	    vkCmdBindDescriptorSets(Cmd.GetCmd(), VK_PIPELINE_BIND_POINT_GRAPHICS, object.Material->PipelineLayout, 0, 1, Gfx::DescriptorsBase::GetInstance()->GetBindlessSet(GetFrameInd()), 0, nullptr);
		vkCmdBindPipeline(Cmd.GetCmd(), VK_PIPELINE_BIND_POINT_GRAPHICS, object.Material->Pipeline);
    }

	Gfx::MeshPushConstants constants;
	constants.texturebind = object.TextureBind[GetFrameInd()];
	constants.bufferbind = object.BufferBind[GetFrameInd()];
      if (object.HasStorage) {
        constants.storagebind = object.StorageBind[GetFrameInd()];
        constants.instancebind = object.InstanceBind[GetFrameInd()];
    }
	vkCmdPushConstants(Cmd.GetCmd(), object.Material->PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Gfx::MeshPushConstants), &constants);

    // why are you binding buffer for every mesh O.o
        #ifdef COMPUTE_SKINNING
        // if (!final_vertex_offsets.empty() && object.Model[0]) {
        if (object.skinned_vertex) {    
     // printf("ooooooaahhhhhhhhhwwwwwwww\n");
    // printf("%d----%d\n", skinning_iterator, final_vertex_offsets.size());
        VkDeviceSize    offset = sizeof(VertexOutputData) * final_vertex_offsets[skinning_iterator];
            skinning_iterator++;
        VkBuffer buffer = Gfx::DescriptorsBase::GetInstance()->GetBuffer(0, SKINNING_OUT);
		vkCmdBindVertexBuffers(Cmd.GetCmd(), 0, 1, &buffer, &offset);
        }
        else if (object.input_vertex) {
		    VkDeviceSize    offset = sizeof(VertexInputData) * united_vertex_offsets[object.Model[0]->Meshes[0]->model_id];
            VkBuffer buffer = Gfx::DescriptorsBase::GetInstance()->GetBuffer(0, SKINNING_IN);
		    vkCmdBindVertexBuffers(Cmd.GetCmd(), 0, 1, &buffer, &offset);
        }
        else if (object.output_vertex) {
		    VkDeviceSize    offset = sizeof(VertexOutputData) * final_mapped_united_vertex_offsets[object.Model[0]->Meshes[0]->model_id];
            VkBuffer buffer = Gfx::DescriptorsBase::GetInstance()->GetBuffer(0, SKINNING_OUT);
		    vkCmdBindVertexBuffers(Cmd.GetCmd(), 0, 1, &buffer, &offset);
        }
        else {
		    VkDeviceSize offset = {0};
		    vkCmdBindVertexBuffers(Cmd.GetCmd(), 0, 1, &mesh->VertexBuffer.Buffer, &offset);
        }
		vkCmdBindIndexBuffer(Cmd.GetCmd(), mesh->IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT16);
        #else
        	if (bindindex) {
		        VkDeviceSize offset = {0};
        		vkCmdBindVertexBuffers(Cmd.GetCmd(), 0, 1, &mesh->VertexBuffer.Buffer, &offset);
        		vkCmdBindIndexBuffer(Cmd.GetCmd(), mesh->IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT16);
        	}
        #endif
	if (ismodel) {
		vkCmdDrawIndexed(Cmd.GetCmd(), static_cast<uint32_t>(mesh->AIindices.size()), 1, 0, 0, InstanceIndex);
        InstanceIndex++;
	}
	else {
		vkCmdDrawIndexed(Cmd.GetCmd(), static_cast<uint32_t>(mesh->Indices.size()), 1, 0, 0, 0);
	}
    Utils::Debug::GetInstance()->DebugDrawCall();
}

void Gfx::Renderer::DrawThreaded(VkCommandBuffer cmd, Gfx::RenderObject& object, Material* mat, Gfx::MeshInfo* mesh, Gfx::MeshPushConstants& constants, bool ismodel, uint32_t inst_ind)
{
	bool bindpipe, bindindex = false;
	CheckTmpBindings(mesh, mat, &bindpipe, &bindindex);

	//if (bindpipe) {
	    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mat->PipelineLayout, 0, 1, Gfx::DescriptorsBase::GetInstance()->GetBindlessSet(GetFrameInd()), 0, nullptr);
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mat->Pipeline );
    //}

		vkCmdPushConstants(cmd, mat->PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Gfx::MeshPushConstants), &constants);

	//if (bindindex) {
		VkDeviceSize offset = {0};
		vkCmdBindVertexBuffers(cmd, 0, 1, &mesh->VertexBuffer.Buffer, &offset);
		vkCmdBindIndexBuffer(cmd, mesh->IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT16);
	//}
	if (ismodel) {
		vkCmdDrawIndexed(cmd, static_cast<uint32_t>(mesh->AIindices.size()), 1, 0, 0, inst_ind);
        //InstanceIndex++;
	}
	else {
		vkCmdDrawIndexed(cmd, static_cast<uint32_t>(mesh->Indices.size()), 1, 0, 0, 0);
	}
    Utils::Debug::GetInstance()->DebugDrawCall();
}

void Gfx::Renderer::Draw(Gfx::RenderObject& object)
{
    bool bindpipe, bindindex = false;
    if (object.Model[GetFrameInd()]) {
		DrawMeshes(object);
	}
	else {
		DrawMesh(object, object.Mesh, false);
	}
}

void Gfx::Renderer::Compute(Gfx::RenderObject& object, uint32_t work_groups, uint32_t work_groups2 )
{
    // Cmd.MemoryBarrier(VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
#ifdef TRACY
     ZoneScopedN("Renderer::PrepareInstanceBuffer::compute");
#endif
	bool bindpipe, bindindex = false;
    Gfx::Material* mat = object.Material;
    CheckTmpBindings(nullptr, mat, &bindpipe, &bindindex);

	if (bindpipe) {
	    vkCmdBindDescriptorSets(Cmd.GetCmd(), VK_PIPELINE_BIND_POINT_COMPUTE, mat->PipelineLayout, 0, 1, Gfx::DescriptorsBase::GetInstance()->GetBindlessSet(GetFrameInd()), 0, nullptr);
		vkCmdBindPipeline(Cmd.GetCmd(), VK_PIPELINE_BIND_POINT_COMPUTE, mat->Pipeline);
    }

	Gfx::MeshPushConstants constants;
    constants.storagebind = object.StorageBind[GetFrameInd()];
    constants.instancebind = object.InstanceBind[GetFrameInd()];
    constants.bufferbind = object.BufferBind[GetFrameInd()];
    // used by compute skinning shader in order to grab the instance buffer
    constants.texturebind = object.TextureBind[GetFrameInd()];
    constants.vertex_in_offset = object.SkinningHelperBind[GetFrameInd()];
    // size of animation compute pass
    constants.inst_index = u_int32_t(Core::Scene::GetInstance()->GetAnimationIndexSize()); 
    bool skip_barrier = false;
    if (object.Model[0]) {
        skip_barrier = true;
        // constants.vertex_in_offset = final_vertex_offsets[skinning_iterator];//united_vertex_offsets[object.Model[0]->Meshes[0]->model_id];
        // constants.vertex_out_offset = final_vertex_offsets[skinning_iterator];
        // constants.inst_index = skinning_iterator;
        // skinning_iterator++;
        //Core::Scene::GetInstance()->GetFinalVertOffset(object.Model[0]->Meshes[0]->model_id);
    }
	vkCmdPushConstants(Cmd.GetCmd(), mat->PipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(Gfx::MeshPushConstants), &constants);

    vkCmdDispatch(Cmd.GetCmd(), work_groups, work_groups2, 1);
//printf("sizeof------%lu|%lu\n", sizeof(glm::vec4), sizeof(glm::vec2));
#ifndef COMPUTE_SKINNING
    if (!skip_barrier)
        Cmd.MemoryBarrier(VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
#endif
}

void Gfx::Renderer::BarrierVertexCompute(int b)
{
    if (b == 0) {
        Cmd.MemoryBarrier(0,0, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    }
    if (b == 1) {
        Cmd.MemoryBarrier(VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
              VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
    }
}

void Gfx::Renderer::ComputeParticles(Gfx::RenderObject& object)
{
    Cmd.MemoryBarrier(VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

	bool bindpipe, bindindex = false;
    Gfx::Material* mat = Gfx::Pipeline::GetInstance()->GetMaterial("particles");
    CheckTmpBindings(nullptr, mat, &bindpipe, &bindindex);

	if (bindpipe) {
	    vkCmdBindDescriptorSets(Cmd.GetCmd(), VK_PIPELINE_BIND_POINT_COMPUTE, mat->PipelineLayout, 0, 1, Gfx::DescriptorsBase::GetInstance()->GetBindlessSet(GetFrameInd()), 0, nullptr);
		vkCmdBindPipeline(Cmd.GetCmd(), VK_PIPELINE_BIND_POINT_COMPUTE, mat->Pipeline);
    }

	Gfx::MeshPushConstants constants;
    constants.storagebind = object.StorageBind[GetFrameInd()];
	vkCmdPushConstants(Cmd.GetCmd(), mat->PipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(Gfx::MeshPushConstants), &constants);

    vkCmdDispatch(Cmd.GetCmd(), u_int32_t(NUM_PARTICLES ) / NUM_PARTICLES_PER_WORKGROUP, 1, 1);
//printf("sizeof------%lu|%lu\n", sizeof(glm::vec4), sizeof(glm::vec2));
    Cmd.MemoryBarrier(VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
}

void Gfx::Renderer::EndRenderName()
{
    Gfx::Vulkan::GetInstance()->EndDebugRenderName(Cmd.GetCmd());
}

void Gfx::Renderer::DebugRenderName(const std::string& str)
{
    static float r, g, b = 0.0f;
    static bool r_passed, g_passed, b_passed = false;

    if (!r_passed) {
        r += 0.5f;
        g += 0.2f;
        b += 0.2f;
        r_passed = true;
        b_passed = false;
        g_passed = false;
    }
    else if (!b_passed) {
        b += 0.5f;
        r += 0.2f;
        g += 0.2f;
        b_passed = true;
    }
    else if (!g_passed) {
        g += 0.5f;
        r += 0.2f;
        b += 0.2f;
        g_passed = true;
        r_passed = false;
    }
    r = r >= 1.0f ? 0.0 : r;
    b = b >= 1.0f ? 0.0 : b;
    g = g >= 1.0f ? 0.0 : g;

    VkDebugUtilsLabelEXT label = {};
    label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    float color[4] = { r, g, b, 1.0f };
    memcpy(label.color, &color[0], sizeof(float) * 4);
    label.pLabelName = str.c_str();
    Gfx::Vulkan::GetInstance()->SetDebugRenderName(Cmd.GetCmd(), &label);

}

void Gfx::Renderer::CheckTmpBindings(Gfx::MeshInfo* mesh, Gfx::Material* material, bool* bindpipe, bool* bindindex)
{
	*bindpipe = TmpBindMaterial != material;
    *bindindex = TmpBindMesh != mesh;
    TmpBindMaterial = *bindpipe ? material : TmpBindMaterial;
	TmpBindMesh = *bindindex ? mesh : TmpBindMesh;
}

void Gfx::Renderer::NullTmpBindings()
{
    TmpBindMaterial = nullptr;
	TmpBindMesh = nullptr;
}

bool Gfx::Renderer::BeginFrame()
{
    Utils::Debug::GetInstance()->NullDrawCall();
    Gfx::Renderer::GetInstance()->NullTmpBindings();

	ImGui::Render();
	
    SwapchainIndex = SyncFrame();
    if (SwapchainIndex == -1 || SwapchainIndex >= MAX_FRAMES + 1) {
        return false;
    }
    Cmd.SetCmd(GetFrame().MainCommandBuffer);
	Cmd.BeginCmd(Cmd.InfoCmd(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT));
   
    return true;
}

void Gfx::Renderer::EndRender()
{
    EndRendering(Cmd.GetCmd());
}

void Gfx::Renderer::StartRender(Gfx::InputAttachments inputs, AttachmentRules rules, bool multithreaded )
{
    Gfx::RenderingAttachmentInfo info;
   	std::vector<RenderingAttachmentInfo> attachmentinfo;
	attachmentinfo.reserve(Gfx::RT_SIZE);
        
    Vector2<int> extents = {(int)Gfx::Device::GetInstance()->GetSwapchainExtent().width, (int)Gfx::Device::GetInstance()->GetSwapchainExtent().height};
	if (inputs.HasColor()) {
        Gfx::RenderingAttachmentInfo info;
		info.IsDepth = false;
		info.Image = GetRT(inputs.GetColor())->GetImage();
        if (((rules & Gfx::ATTACHMENT_RULE_LOAD) == Gfx::ATTACHMENT_RULE_LOAD)) {
            info.Layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
        info.ImageView = GetRT(inputs.GetColor())->GetImageView();
        info.Rules = rules;
		attachmentinfo.push_back(info);
    }
    if (inputs.HasAlbedo()) {
		info.IsDepth = false;
        if ((rules & Gfx::ATTACHMENT_RULE_LOAD) == Gfx::ATTACHMENT_RULE_LOAD) {
            info.Layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }

        for (int i = 0; i < GBUFFER_RT_SIZE; i++) {
            Gfx::RenderTargetsList id = static_cast<Gfx::RenderTargetsList>(Gfx::RT_ALBEDO + i);
            info.Image = GetRT(id)->GetImage();
            info.ImageView = GetRT(id)->GetImageView();
            attachmentinfo.push_back(info);
        }
    }
	if (inputs.HasDepth()) {
        if (inputs.GetDepth() == Gfx::RT_SHADOWS) {
            extents = RTs[Gfx::RT_SHADOWS]->GetSize();
        }
        Gfx::RenderingAttachmentInfo info;
		info.IsDepth = true;
        if ((rules & Gfx::ATTACHMENT_RULE_LOAD) == Gfx::ATTACHMENT_RULE_LOAD) {
            info.Layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        }
		info.Image = GetRT(inputs.GetDepth())->GetImage();
        info.ImageView = GetRT(inputs.GetDepth())->GetImageView();
        attachmentinfo.push_back(info);
	}

    VkRenderingAttachmentInfoKHR depthinfo = {};
    std::vector<VkRenderingAttachmentInfoKHR> infos;
	const VkRenderingInfo& renderinfo = Cmd.GetRenderingInfo(attachmentinfo, infos, depthinfo, extents , multithreaded);
    BeginRendering(Cmd.GetCmd(), &renderinfo);
}
void Gfx::Renderer::TransferLayoutsDebug()
{
    GetRT(Gfx::RT_ALBEDO)->MemoryBarrier(Cmd.GetCmd(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    GetRT(Gfx::RT_POSITION)->MemoryBarrier(Cmd.GetCmd(),VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    GetRT(Gfx::RT_NORMAL)->MemoryBarrier(Cmd.GetCmd(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  //  GetRT(Gfx::RT_SHADOWS)->MemoryBarrier(Cmd.GetCmd(), VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void Gfx::Renderer::RenderUI()
{
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), Cmd.GetCmd());
}

void Gfx::Renderer::CopyImage(Gfx::RenderTargetsList src_id, Gfx::RenderTargetsList dst_id)
{
    Cmd.CopyImage(	GetRT(src_id)->GetImage(),
					GetRT(src_id)->GetSize(),
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					GetRT(dst_id)->GetImage(),
					GetRT(dst_id)->GetSize(),
                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	Cmd.MemoryBarrier(GetRT(dst_id)->GetImage(),
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					Cmd.GetSubresourceMainRange());

    Cmd.MemoryBarrier(GetRT(src_id)->GetImage(),
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					Cmd.GetSubresourceMainRange());


}

void Gfx::Renderer::EndFrame()
{
	Cmd.CopyImage(	GetRT(Gfx::RT_MAIN_COLOR)->GetImage(),
					GetRT(Gfx::RT_MAIN_COLOR)->GetSize(),
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					Gfx::Device::GetInstance()->GetSwapchainImage(SwapchainIndex),
					{(int)Gfx::Device::GetInstance()->GetSwapchainExtent().width, (int)Gfx::Device::GetInstance()->GetSwapchainExtent().height},
					VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	Cmd.MemoryBarrier(Gfx::Device::GetInstance()->GetSwapchainImage(SwapchainIndex),
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
					Cmd.GetSubresourceMainRange());
    
#ifdef TRACY
   TracyVkCollect(Gfx::Renderer::GetInstance()->GetTracyContext(), Gfx::Renderer::GetInstance()->GetCmd());
#endif
    Cmd.EndCmd();

    VkSubmitInfo submit = {};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.pNext = nullptr;
	VkPipelineStageFlags waitstage2 = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submit.pWaitDstStageMask = &waitstage2;
	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &GetFrame(PrevSwapchainIndex).PresentSemaphore;
	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &GetFrame(SwapchainIndex).RenderSemaphore;
	submit.commandBufferCount = 1;
	VkCommandBuffer cmd = Cmd.GetCmd();
	submit.pCommandBuffers = &cmd;
	VK_ASSERT(vkQueueSubmit(Gfx::Device::GetInstance()->GetQueue(Gfx::GRAPHICS_QUEUE), 1, &submit, GetFrame(SwapchainIndex).RenderFence), "failed to submit queue!");

	VkResult presentresult = Cmd.Present(
		Gfx::Device::GetInstance()->GetQueue(Gfx::GRAPHICS_QUEUE),
		Cmd.PresentInfo(
			&Gfx::Device::GetInstance()->GetSwapchain(),
			&GetFrame(SwapchainIndex).RenderSemaphore,
			&SwapchainIndex
		)
	);

	if (presentresult == VK_ERROR_OUT_OF_DATE_KHR) {
       OnResize = true;
	}

   SetFrameInd();
}
Gfx::RenderTarget* Gfx::Renderer::GetCubemap(const std::string& name)
{
	CubemapsMap::iterator it = Cubemaps.find(name);
	if (it == Cubemaps.end()) {
		return nullptr;
	}
	else {
		return &(*it).second;
	}
}
Gfx::RenderTarget* Gfx::Renderer::GetCubemapImgui(const std::string& name)
{
	TexturesMap::iterator it = CubemapsImgui.find(name);
	if (it == CubemapsImgui.end()) {
		return nullptr;
	}
	else {
		return &(*it).second;
	}
}

Gfx::RenderTarget* Gfx::Renderer::GetTexture(const std::string& name)
{
	TexturesMap::iterator it = Textures.find(name);
	if (it == Textures.end()) {
		return nullptr;
	}
	else {
		return &(*it).second;
	}
}

std::string time_domain_to_string(VkTimeDomainEXT input_time_domain)
{
	switch (input_time_domain)
	{
		case VK_TIME_DOMAIN_DEVICE_EXT:
			return "device time domain";
		case VK_TIME_DOMAIN_CLOCK_MONOTONIC_EXT:
			return "clock monotonic time domain";
		case VK_TIME_DOMAIN_CLOCK_MONOTONIC_RAW_EXT:
			return "clock monotonic raw time domain";
		case VK_TIME_DOMAIN_QUERY_PERFORMANCE_COUNTER_EXT:
			return "query performance time domain";
		default:
			return "unknown time domain";
	}
}
void Gfx::Renderer::InitTracy()
{
	for (int i = 0; i < MAX_FRAMES; i++) {
        Tracy.Context[i] = TracyVkContextCalibrated(Gfx::Device::GetInstance()->GetPhysicalDevice(), Gfx::Device::GetInstance()->GetDevice(), Gfx::Device::GetInstance()->GetQueue(GRAPHICS_QUEUE), Tracy.Cmd, Tracy.GetPhysicalDeviceCalibrateableTimeDomainsEXT, Tracy.GetCalibratedTimestampsEXT );
        char buf[50];
        int n = sprintf(buf, "Vulkan Context [%d]", i);
        TracyVkContextName(Tracy.Context[i], buf, n); 
    }
}

void Gfx::Renderer::DestroyTracy()
{
    for (int i = 0; i < MAX_FRAMES; i++) {
            TracyVkDestroy(Tracy.Context[i]);
        }

}

void Gfx::Renderer::PrepareCompute()
{
    std::uniform_real_distribution<float> uniform(-1.0f, 1.0f);
    std::mt19937 engine;
    for (unsigned i = 0; i < NUM_PARTICLES; i++) {
            CompData.position[i] = glm::vec2(0.2f * uniform(engine), 0.2f * uniform(engine));
            float velocity = 0.008f + 0.003f * uniform(engine);
            float angle = 100.0f * uniform(engine);
            CompData.velocity[i] = (velocity * glm::vec2(glm::cos(angle), glm::sin(angle)));
            float y = 0.8f + 0.2f * uniform(engine);
            float saturation = 0.8f + 0.2f * uniform(engine);
            float hue = 100.0f * uniform(engine);
            float u = saturation * glm::cos(hue);
            float v = saturation * glm::sin(hue);
            glm::vec3 rgb = glm::mat3(1.0f, 1.0f, 1.0f, 0.0f, -0.39465f, 2.03211f, 1.13983f, -0.58060f, 0.0f) * glm::vec3(y, u, v);
            CompData.color[i] = glm::vec4(rgb, 0.4f);
    }

    const size_t buffersize = (sizeof(ComputeData));
    BufferHelper::MapMemory(Gfx::DescriptorsBase::GetInstance()->GetComputeUBO(GetFrameInd()), buffersize, 0, &CompData);
}

void Gfx::Renderer::PrepareStorageBuffer()
{
#ifdef TRACY
    ZoneScopedN("Renderer::PrepareStorageBuffer");
#endif
    /*if (!Core::WindowManager::GetInstance()->IsMousePressed()) {*/
    /*    u_int dst[DEPTH_ARRAY_SCALE] = {0};*/
    /*    BufferHelper::MapMemory(Gfx::DescriptorsBase::GetInstance()->GetStorageUBO(), sizeof(u_int) * DEPTH_ARRAY_SCALE, 0, dst);*/
    /*    //SelectedID = 0;*/
    /*    return;*/
    /*}*/

    //for (int i = 0; i < MAX_INSTANCES; i++) {


    int selectedID = -1;

       // TODO: improve pressision
   // printf("--------------------\n");
	if (!Core::WindowManager::GetInstance()->IsMouseSelected() && !Core::WindowManager::GetInstance()->IsMouseMove()) { void* storage;
    VkDeviceSize storagesize = sizeof(uint32_t) * DEPTH_ARRAY_SCALE;
    vkMapMemory(Gfx::Device::GetInstance()->GetDevice(),Gfx::DescriptorsBase::GetInstance()->GetStorageBufferMemory(GetFrameInd()), 0, storagesize, 0, (void**)&storage);


     
    uint32_t* u = static_cast<uint32_t*>(storage);
    for (int i = 0; i < DEPTH_ARRAY_SCALE; i++) {
        if (u[i] != 0) {
            selectedID = u[i];
			Core::Scene::GetInstance()->SetSelectedID(selectedID);
          // printf("[%d][%d] \n ",i, u[i]);
			/* 		 uint32_t dst[DEPTH_ARRAY_SCALE] = {0};*/
			/*memcpy(storage, dst, DEPTH_ARRAY_SCALE * sizeof(uint32_t));*/
			/*vkUnmapMemory(Gfx::Device::GetInstance()->GetDevice(),Gfx::DescriptorsBase::GetInstance()->GetStorageBufferMemory());*/


            break;
        }
    }

		  if (selectedID == -1) {
		Core::Scene::GetInstance()->SetSelectedID(0);
		  //printf("-----|%d|\n\n", selectedID);

		  } uint32_t dst[DEPTH_ARRAY_SCALE] = {0};
    memset(dst, 0, DEPTH_ARRAY_SCALE * sizeof(uint32_t));
    memcpy(storage, dst, DEPTH_ARRAY_SCALE * sizeof(uint32_t));
    vkUnmapMemory(Gfx::Device::GetInstance()->GetDevice(),Gfx::DescriptorsBase::GetInstance()->GetStorageBufferMemory(GetFrameInd()));

   // Core::WindowManager::GetInstance()->ReleaseMouseSelected();

 //   Core::WindowManager::GetInstance()->ReleaseMouseSelected();
  
	}
    else {
        void* storage;

    VkDeviceSize storagesize = sizeof(uint32_t) * DEPTH_ARRAY_SCALE;
    vkMapMemory(Gfx::Device::GetInstance()->GetDevice(),Gfx::DescriptorsBase::GetInstance()->GetStorageBufferMemory(GetFrameInd()), 0, storagesize, 0, (void**)&storage);

uint32_t dst[DEPTH_ARRAY_SCALE] = {0};
    memset(dst, 0, DEPTH_ARRAY_SCALE * sizeof(uint32_t));
    memcpy(storage, dst, DEPTH_ARRAY_SCALE * sizeof(uint32_t));
    vkUnmapMemory(Gfx::Device::GetInstance()->GetDevice(),Gfx::DescriptorsBase::GetInstance()->GetStorageBufferMemory(GetFrameInd()));


    }

 
   // printf("-----|%d|---\n", Core::Scene::GetInstance()->GetSelectedID());
	//}

    //printf("\n\n");


}
#ifdef COMPUTE_SKINNING
void Gfx::Renderer::FillSkinningBuffer()
{

    // update global anim buffer bind 
    //
    // global_animation_bind = Gfx::DescriptorsBase::GetInstance()->UpdateCompute(Gfx::DescriptorsBase::GetInstance()->GetAnimationBuffer(0), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, Gfx::DescriptorsBase::GetInstance()->GetAnimationUBO(0).tag, 0);

    Modules::ScenesMap& map =  Core::Scene::GetInstance()->GetScenes();
    // Modules::Module& module = map[Core::Scene::GetInstance()->GetCurrentScene()];
    Modules::Module& module = map["gbuffer"];
   
    size_t scene_size = Core::Scene::GetInstance()->GetSceneVertSize();
    Gfx::Model* model = Gfx::Model::GetInstance();
    void*data;
    int size = model->GetVerteciesSize();
    size_t buffersize = sizeof(VertexInputData) * size;//size;
    // if (buffersize == 0) {
        // return;
    // }
    BufferHelper::Buffer stagingbuffer;
    BufferHelper::AllocateBuffer(stagingbuffer, buffersize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkMapMemory(Gfx::Device::GetInstance()->GetDevice(), stagingbuffer.DeviceMemory, 0, buffersize, 0, (void**)&data);
    
    VertexInputData* datas = (VertexInputData*)data;
    {
    int vert_offset = 0;
    int final_vert_size = 0;
    int buf_ind = 0;
    int model_id = 0;
    skinning_iterator = 0;
    // for (auto& m : module.GetRenderQueueMap()) {
    for ( auto& it : model->GetNonConstModels()) {
        // for (RenderObject& o : m.second) {
            
            it.second.Meshes[0]->model_id = model_id;    
            united_vertex_offsets[model_id] = vert_offset;
            final_vert_size = 0;
            for (Vertex v : it.second.Meshes[0]->Vertices) {
                datas[buf_ind].datas[0] = vert_offset;
                datas[buf_ind].vposition = v.position;
                datas[buf_ind].vboneid = glm::vec4(v.boneID[0],v.boneID[1],v.boneID[2],v.boneID[3]);
                datas[buf_ind].vcolor = glm::vec4(v.color, 1.0);
                datas[buf_ind].vnormal = glm::vec4(v.normal, 1.0);
                datas[buf_ind].vuv = glm::vec4(v.uv, 1.0f,1.0f);
                datas[buf_ind].vweight = glm::vec4(v.weights[0],v.weights[1],v.weights[2],v.weights[3]);
                
                datas[buf_ind].datas[1] = it.second.Meshes[0]->Vertices.size();//it.second.Meshes[i]->Vertices.size();//it.second.MeshBase[i];
                datas[buf_ind].datas[2] = skinning_iterator; 
                // datas[buf_ind].datas1 = glm::vec4(1.0);
                final_vert_size++;
                buf_ind++;
            }
            skinning_iterator++;
        // }
        model_id++;
        vert_offset += final_vert_size;
    }
    }

    vkUnmapMemory(Gfx::Device::GetInstance()->GetDevice(), stagingbuffer.DeviceMemory);

    Submit([&](VkCommandBuffer cmd) {
            VkBufferCopy copy{};
            copy.dstOffset = 0;
            copy.srcOffset = 0;
            copy.size = buffersize;
            for (int i = 0 ; i < 1; i++) {
                vkCmdCopyBuffer(cmd, stagingbuffer.Buffer, Gfx::DescriptorsBase::GetInstance()->GetBuffer(i, SKINNING_IN), 1, &copy );
            }
        });    
    vkDestroyBuffer(Gfx::Device::GetInstance()->GetDevice(), stagingbuffer.Buffer, nullptr);
    vkFreeMemory(Gfx::Device::GetInstance()->GetDevice(), stagingbuffer.DeviceMemory, nullptr);
    

    buffersize = sizeof(VertexOutputHelper) * scene_size;//size;
    BufferHelper::AllocateBuffer(stagingbuffer, buffersize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void*data1;
    vkMapMemory(Gfx::Device::GetInstance()->GetDevice(), stagingbuffer.DeviceMemory, 0, buffersize, 0, (void**)&data1);
    
    VertexOutputHelper* datas2 = (VertexOutputHelper*)data1;
    {
    int vert_offset = 0;
    int final_vert_size = 0;
    int buf_ind = 0;
    int model_id = 0;
    skinning_iterator = 0;
    int instances_tmp = 0;
    // for (auto& m : module.GetRenderQueueMap()) {
        // for (RenderObject& o : m.second) {
        for (const RenderObject& o : module.GetSkinningRQ()) {
            // o.Model[0]->Meshes[0]->model_id = model_id;    
            // united_vertex_offsets[model_id] = vert_offset;
            final_vert_size = 0;
            for (Vertex v : o.Model[0]->Meshes[0]->Vertices) {
                // datas[buf_ind].datas[0] = vert_offset;
                if (o.Spawn)
                    datas2[buf_ind].data = glm::vec4(1, final_vert_size, united_vertex_offsets[o.Model[0]->Meshes[0]->model_id], skinning_iterator);
                else {
                    datas2[buf_ind].data = glm::vec4(1, final_vert_size, united_vertex_offsets[o.Model[0]->Meshes[0]->model_id], instances_tmp);
                }
                final_vert_size++;
                buf_ind++;
            }
            if (o.Spawn) {
                instances_tmp += o.instance_size;
            }
            else {
                instances_tmp++;
            }
            skinning_iterator++;
        }
        model_id++;
        vert_offset += final_vert_size;
    // }
    }

    vkUnmapMemory(Gfx::Device::GetInstance()->GetDevice(), stagingbuffer.DeviceMemory);

    Submit([&](VkCommandBuffer cmd) {
            VkBufferCopy copy{};
            copy.dstOffset = 0;
            copy.srcOffset = 0;
            copy.size = buffersize;
            for (int i = 0 ; i < 1; i++) {
                vkCmdCopyBuffer(cmd, stagingbuffer.Buffer, Gfx::DescriptorsBase::GetInstance()->GetBuffer(i, SKINNING_HELPER), 1, &copy );
            }
        });    
    vkDestroyBuffer(Gfx::Device::GetInstance()->GetDevice(), stagingbuffer.Buffer, nullptr);
    vkFreeMemory(Gfx::Device::GetInstance()->GetDevice(), stagingbuffer.DeviceMemory, nullptr);
    
   {
    buffersize = sizeof(VertexOutputData) * scene_size;//size;
    BufferHelper::AllocateBuffer(stagingbuffer, buffersize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void*data1;
    vkMapMemory(Gfx::Device::GetInstance()->GetDevice(), stagingbuffer.DeviceMemory, 0, buffersize, 0, (void**)&data1);
    
    VertexOutputData* datas2 = (VertexOutputData*)data1;
    {
    int vert_offset = 0;
    int final_vert_size = 0;
    int buf_ind = 0;
    int model_id = 0;
    skinning_iterator = 0;
    // for (auto& m : module.GetRenderQueueMap()) {
        // for (RenderObject& o : m.second) {
        for (const RenderObject& o : module.GetSkinningRQ()) {
            // o.Model[0]->Meshes[0]->model_id = model_id;    
            // united_vertex_offsets[model_id] = vert_offset;
            final_vert_size = 0;
            for (Vertex v : o.Model[0]->Meshes[0]->Vertices) {
                // datas[buf_ind].datas[0] = vert_offset;
                // datas2[buf_ind].data = glm::vec4(1, final_vert_size, united_vertex_offsets[o.Model[0]->Meshes[0]->model_id], skinning_iterator);
                // datas2[buf_ind].vuv = glm::vec4(1.0f, 1.0f, final_vert_size, united_vertex_offsets[o.Model[0]->Meshes[0]->model_id]);
                datas2[buf_ind].vnormal = glm::vec4(v.normal, 1.0);
                datas2[buf_ind].vcolor = glm::vec4(v.color, 1.0);
                datas2[buf_ind].vuv = glm::vec4(v.uv, 1.0f, 1.0);
                // datas[buf_ind].vboneid = glm::vec4(v.boneID[0],v.boneID[1],v.boneID[2],v.boneID[3]);
                // datas[buf_ind].vcolor = glm::vec4(v.color, 1.0);
                // datas[buf_ind].vnormal = glm::vec4(v.normal, 1.0);
                // datas[buf_ind].vuv = glm::vec4(v.uv, 1.0f,1.0f);
                // datas[buf_ind].vweight = glm::vec4(v.weights[0],v.weights[1],v.weights[2],v.weights[3]);
                //
                // datas[buf_ind].datas[1] = o.Model[0]->Meshes[0]->Vertices.size();//it.second.Meshes[i]->Vertices.size();//it.second.MeshBase[i];
                // datas[buf_ind].datas[2] = skinning_iterator; 
                // datas[buf_ind].datas1 = glm::vec4(1.0);
                final_vert_size++;
                buf_ind++;
            }
            skinning_iterator++;
        }
        model_id++;
        vert_offset += final_vert_size;
    // }
    }

    vkUnmapMemory(Gfx::Device::GetInstance()->GetDevice(), stagingbuffer.DeviceMemory);

    Submit([&](VkCommandBuffer cmd) {
            VkBufferCopy copy{};
            copy.dstOffset = 0;
            copy.srcOffset = 0;
            copy.size = buffersize;
            for (int i = 0 ; i < 1; i++) {
                vkCmdCopyBuffer(cmd, stagingbuffer.Buffer, Gfx::DescriptorsBase::GetInstance()->GetBuffer(i, SKINNING_OUT), 1, &copy );
            }
        });    
    vkDestroyBuffer(Gfx::Device::GetInstance()->GetDevice(), stagingbuffer.Buffer, nullptr);
    vkFreeMemory(Gfx::Device::GetInstance()->GetDevice(), stagingbuffer.DeviceMemory, nullptr);
    

    }
    // }
    skinning_iterator = 0;
    final_vertex_offsets.clear();
    int vert_count = 0;
    int iter = 0;
    // for (auto& m : module.GetRenderQueueMap()) {
        // for (RenderObject& o : m.second) {
        for (const RenderObject& o : module.GetSkinningRQ()) {
            final_vertex_offsets.push_back(vert_count);
            final_mapped_united_vertex_offsets[o.Model[0]->Meshes[0]->model_id] = final_vertex_offsets[iter];
            vert_count += o.Model[0]->Meshes[0]->Vertices.size();
            //if ()
            iter++;
        }
    // }

}
#endif
void Gfx::Renderer::PrepareInstanceBuffer()
{
    skinning_iterator = 0;
    Modules::ScenesMap& map =  Core::Scene::GetInstance()->GetScenes();
    Modules::Module& modulegizmo = map["gizmo"];
    Modules::Module& module = map[Core::Scene::GetInstance()->GetCurrentScene()];
    void* instancedata;
#ifdef TRACY
    ZoneNamedN(Zone1, "Renderer::PrepareInstanceBuffer", true);
#endif
    {
#ifdef TRACY
    ZoneNamedN(Zone2, "Renderer::PrepareInstanceBuffer::MapMemory", true);
#endif
        const size_t buffersize = sizeof(InstanceData) * MAX_INSTANCES ;
        vkMapMemory(Gfx::Device::GetInstance()->GetDevice(),Gfx::DescriptorsBase::GetInstance()->GetInstanceBufferMemory(GetFrameInd()), 0, buffersize, 0, (void**)&instancedata);
    }
    {
#ifdef TRACY
    ZoneNamedN(Zone3, "Renderer::PrepareInstanceBuffer::OldCode-DeadCode", true);
#endif
        u_int32_t obj_size = module.GetRenderQueue(Modules::RQ_GENERAL).size();
        uint32_t inst_ind = 0;//Gfx::Renderer::GetInstance()->GetInstanceInd();
        //
        std::vector<uint32_t> num_obj_per_thread(Thread::MAX_RENDER_THREAD_NUM, (uint32_t)module.GetRenderQueue(Modules::RQ_GENERAL).size() / Thread::MAX_RENDER_THREAD_NUM );
        //num_obj_per_thread = { (uint32_t)module.GetRenderQueue().size() / Thread::MAX_THREAD_NUM };

        bool iseven = (module.GetRenderQueue(Modules::RQ_GENERAL).size() % Thread::MAX_RENDER_THREAD_NUM) == 0;
        if (!iseven) {
            num_obj_per_thread[num_obj_per_thread.size() - 1] += (module.GetRenderQueue(Modules::RQ_GENERAL).size() % Thread::MAX_RENDER_THREAD_NUM);
        }
        /*for (uint32_t o : num_obj_per_thread) {*/
        /**/
        /*    printf("NUM OBJ PER thread === %d\n", o);*/
        /*}*/
        /*    printf("\n------------== size %d\n ------------\n", obj_size);*/
        u_int32_t first_obj_size = 0;
        u_int32_t sec_obj_size = 0;// module.GetRenderQueue().size() / 2;

        uint32_t fin_inst_ind = 0;
        uint32_t fin_inst_ind2 = 0;


        first_obj_size = 0;
        sec_obj_size = 0;
        uint32_t inst = 0;

    }
        int i = 0;
        InstanceData* datas = (InstanceData*)instancedata ;//Gfx::DescriptorsBase::GetInstance()->GetMappedInstanceData();
    {
#ifdef TRACY
    ZoneNamedN(Zone3, "Renderer::PrepareInstanceBuffer::OldCode-DeadCode", true);
#endif
        bool hasanim = false;
        for (auto& it : module.GetRenderQueueMap()) {
        for (Gfx::RenderObject& obj : it.second) {
            
            if (!obj.Model[GetFrameInd()]) {
                i++;
                continue;
            }
            hasanim = obj.HasAnimation;//Core::Scene::GetInstance()->HasAnimation(obj.ID);
            for (int j = 0; j < obj.Model[GetFrameInd()]->Meshes.size(); j++ ) {
             //     if (Thread::Pool::GetInstance()->IsInit()) {
             //     Thread::Pool::GetInstance()->Push({ Thread::Task::Name::RENDER, Thread::Task::Type::EXECUTE,
             // {}, [this, hasanim, &obj, &datas, i]() {
            //
//                 {
// #ifdef TRACY
//                 ZoneNamedN(Zone4, "Renderer::PrepareInstanceBuffer::Loop-get-bones", true);
// #endif
#ifndef COMPUTE_MTX
                    // if (hasanim) {
                    //     for (int k = 0; k < obj.Model[GetFrameInd()]->Bones.Info.size(); k++) {
                    //         datas[i].bonesmatrices[k] = obj.Model[GetFrameInd()]->Bones.Info[k].FinTransform;
                    //     }
                    // }
#endif
                    if (hasanim) {
                        for (int k = 0; k < 200; k++) {
                            datas[i].anim_transforms[k] = glm::mat4(1);//obj.Model[GetFrameInd()]->Bones.Info[k].FinTransform;
                        }
                        datas[i].anim_ind = Core::Scene::GetInstance()->GetAnimationIndex(obj.SpawnName); 
                    }

//                 }
                {
#ifdef TRACY
                ZoneNamedN(Zone5, "Renderer::PrepareInstanceBuffer::Loop-get-mtx-inst", true);
#endif
                    datas[i].hasanimation = hasanim ? 1 : 0;
                    datas[i].position = glm::vec4(obj.Position.convert(), 1.0f);
                    datas[i].rotation = glm::mat4(1); 
                    if (obj.rotation.x > 0 || obj.rotation.y > 0 || obj.rotation.z > 0) {
                    glm::mat rot = glm::mat4(1);
                    rot = glm::rotate(glm::mat4(1), glm::radians(obj.rotation.x), glm::vec3(1,0,0));
                    rot = glm::rotate(rot, glm::radians(obj.rotation.y), glm::vec3(0,1,0));
                    datas[i].rotation = glm::rotate(rot, glm::radians(obj.rotation.z), glm::vec3(0,0,1));
                        }
                    datas[i].scale = glm::scale(glm::mat4(1), glm::vec3(obj.scale));
                    datas[i].gizmo_dist = glm::vec4(1.0);
// #ifndef COMPUTE_MTX
                    datas[i].rendermatrix =  glm::translate(glm::mat4(1.0f), glm::vec3(obj.Position.x, obj.Position.y, obj.Position.z));
// #endif
	                datas[i].texturebind = obj.TextureBind[GetFrameInd()];
	                datas[i].bufferbind = obj.BufferBind[GetFrameInd()];
	    //    da    tas[i].boneID = -1;
	                if (obj.HasStorage) {
	                    datas[i].storagebind = obj.StorageBind[GetFrameInd()];
	                    datas[i].objectID = obj.ID;
	                    datas[i].selected = (obj.IsSelected || obj.ID == Core::Scene::GetInstance()->GetSelectedID()) ? 1 : 0;
	                    datas[i].boneID = Utils::Debug::GetInstance()->Bones ? Utils::Debug::GetInstance()->BoneID : 0;
	                    datas[i].gizmo = obj.GizmoType;
	                }
	            }
                // }, {}, 0, nullptr, {} });
                // }
                i++;
            }
        }
        // if (Thread::Pool::GetInstance()->IsInit()) {
        //     Thread::Pool::GetInstance()->WaitWork();
        // }

        }
        for (Gfx::RenderObject& obj : modulegizmo.GetRenderQueue(Modules::RQ_GENERAL)) {
             if (!obj.Model[GetFrameInd()] || !obj.IsVisible) continue;
            for (int j = 0; j < obj.Model[GetFrameInd()]->Meshes.size(); j++ ) {
                
                {
#ifdef TRACY
                ZoneNamedN(Zone6, "Renderer::PrepareInstanceBuffer::Gizmo-mtx-inst", true);
#endif
                    float dist = glm::distance(glm::vec3(CamData.viewpos.x, CamData.viewpos.y, CamData.viewpos.z), glm::vec3(obj.Position.x, obj.Position.y, obj.Position.z) )* 0.05;
                    if (dist <= 0.5) {
                        dist = 0.5;
                    }

                    glm::vec3 distfin = glm::vec3(dist);

                    datas[i].gizmo_dist = glm::vec4(distfin, 1.0f);
                    datas[i].position = glm::vec4(obj.Position.convert(), 1.0f);
                    datas[i].rotation = glm::mat4(1); 
                    datas[i].scale = glm::mat4(1); 
// #ifndef COMPUTE_MTX
                    datas[i].rendermatrix = glm::translate(glm::mat4(1.0f), glm::vec3(obj.Position.x, obj.Position.y, obj.Position.z));
                    datas[i].rendermatrix = glm::scale(datas[i].rendermatrix, distfin);
// #endif
                    datas[i].texturebind = obj.TextureBind[GetFrameInd()];
	                datas[i].bufferbind = obj.BufferBind[GetFrameInd()];
	                if (obj.HasStorage) {
	                    datas[i].storagebind = obj.StorageBind[GetFrameInd()];
	                    datas[i].objectID = obj.ID;
	                    datas[i].selected = (obj.IsSelected || obj.ID == Core::Scene::GetInstance()->GetSelectedID()) ? 1 : 0;
	                    datas[i].boneID = Utils::Debug::GetInstance()->Bones ? Utils::Debug::GetInstance()->BoneID : 0;
	                    datas[i].gizmo = obj.GizmoType;
	                }
                }
                i++;
            }

        }
    }
    InstanceCount = i;
    InstanceIndex = 0;
    {
#ifdef TRACY
     ZoneNamedN(Zone7, "Renderer::PrepareInstanceBuffer::UnmapMemory", true);
#endif
        vkUnmapMemory(Gfx::Device::GetInstance()->GetDevice(),Gfx::DescriptorsBase::GetInstance()->GetInstanceBufferMemory(GetFrameInd()));
    }

#ifdef COMPUTE_MTX

    if (Utils::IsBitSet(Engine::GetInstance()->GetState(), ENGINE_STATE_PLAY)) {
    void* animdata;
    {
#ifdef TRACY
        ZoneNamedN(Zone8, "Renderer::PrepareInstanceBuffer::Mapmem-anim", true);
#endif
        const size_t buffersize = sizeof(AnimationComputeData) * Core::Scene::GetInstance()->GetAnimationIndexSize();//sizeof(AnimationComputeData) * MAX_INSTANCES ;
        vkMapMemory(Gfx::Device::GetInstance()->GetDevice(),Gfx::DescriptorsBase::GetInstance()->GetAnimationBufferMemory(GetFrameInd()), 0, buffersize, 0, (void**)&animdata);
    }
    AnimationComputeData* animdatas = (AnimationComputeData*)animdata;//Gfx::DescriptorsBase::GetInstance()->GetMappedAnimationData();
    bool hasanim = false;
    i = 0;
    int anim_ind = 0;
    {
#ifdef TRACY
        ZoneNamedN(Zone9, "Renderer::PrepareInstanceBuffer::iterate-anim", true);
#endif
        Modules::Module& module_anim = map["gbuffer"];
        // for (auto& it : module.GetRenderQueueMap()) {
            for (Gfx::RenderObject& obj : module_anim.GetSkinningRQ()) {
                {
#ifdef TRACY
                    ZoneNamedN(Zone12, "Renderer::PrepareInstanceBuffer::check-for-anim", true);
#endif

                    hasanim = obj.HasAnimation;//Core::Scene::GetInstance()->HasAnimation(obj.ID);
                    if (!hasanim || !obj.Model[GetFrameInd()]) {
                        i++;
                        continue;
                    }

                }

                {
#ifdef TRACY
                    ZoneNamedN(Zone13, "Renderer::PrepareInstanceBuffer::for-mesh-loop", true);
#endif


                    const Core::aiSceneInfo& scene = Core::Scene::GetInstance()->GetInfo(obj.Model[GetFrameInd()], obj.SpawnName, obj.AnimOffset); 
                    for (int k = 0; k < obj.Model[GetFrameInd()]->Meshes.size(); k++ ) {
                    if (hasanim) {
                        {
#ifdef TRACY
                            ZoneNamedN(Zone11, "Renderer::PrepareInstanceBuffer::in-loop-processanim", true);
#endif
                            AnimationComputeData& animdat = animdatas[anim_ind];
                            animdat.rootssize = scene.rootssize;
                            animdat.timetick = scene.timeticks;
                            animdat.animsize = scene.animsize;
                            animdat.instance_buffer_ind = i;//scene.animsize;

#ifndef MEMCPY_TEST
                            // memcpy(animdat.animisempty, scene.floats.animisempty, sizeof(int) * 118);
                            memcpy(animdat.pos_factor, scene.floats.pos_factor, sizeof(float) * 118);
                            memcpy(animdat.scale_factor, scene.floats.scale_factor, sizeof(float) * 118);
                            memcpy(animdat.rot_factor, scene.floats.rot_factor, sizeof(float) * 118);
                            // memcpy(animdat.pos_comp, scene.floats.pos_comp, sizeof(int) * 118);
                            // memcpy(animdat.scale_comp, scene.floats.scale_comp, sizeof(int) * 118);
                            // memcpy(animdat.rot_comp, scene.floats.rot_comp, sizeof(int) * 118);
                            // memcpy(animdat.pos_out, scene.matricies.pos_out, sizeof(glm::vec4) * 118);
                            memcpy(animdat.pos_start, scene.matricies.pos_start, sizeof(glm::vec4) * 118);
                            memcpy(animdat.pos_end, scene.matricies.pos_end, sizeof(glm::vec4) * 118);
                            // memcpy(animdat.scale_out, scene.matricies.scale_out, sizeof(glm::vec4) * 118);
                            memcpy(animdat.scale_start, scene.matricies.scale_start, sizeof(glm::vec4) * 118);
                            memcpy(animdat.scale_end, scene.matricies.scale_end, sizeof(glm::vec4) * 118);
                            // memcpy(animdat.rot_out, scene.matricies.rot_out, sizeof(glm::mat4) * 118);
                            memcpy(animdat.rot_start, scene.matricies.rot_start, sizeof(glm::mat4) * 118);
                            memcpy(animdat.rot_end, scene.matricies.rot_end, sizeof(glm::mat4) * 118);
                            // memcpy(animdat.animrot, scene.animrot, sizeof(glm::mat4) * 118);

                            memcpy(animdat.nodeAnimInd, scene.floats.nodeAnimInd, sizeof(int) * 118);
                            memcpy(animdat.nodeBoneInd, scene.floats.nodeBoneInd, sizeof(int) * 118);
                            memcpy(animdat.nodeIndex, scene.floats.nodeIndex, sizeof(int) * 118);
                            // memcpy(animdat.nodesettransform, scene.floats.nodesettransform, sizeof(int) * 118);
                            memcpy(animdat.nodeOffset, scene.matricies.nodeOffset, sizeof(glm::mat4) * 118);
                            memcpy(animdat.nodeTransform, scene.matricies.nodeTransform, sizeof(glm::mat4) * 118);
#else

                            {
#ifdef TRACY
                            ZoneNamedN(Zone19, "Renderer::PrepareInstanceBuffer::in-loop-matricies", true);
#endif
                            memcpy(&animdat.matricies, &scene.matricies, sizeof(AnimMatricies));
                            }
                            {
#ifdef TRACY
                                ZoneNamedN(Zone18, "Renderer::PrepareInstanceBuffer::in-loop-floats", true);
#endif

                                memcpy(&animdat.floats, &scene.floats, sizeof(AnimFloats));
                            }
#endif
                            anim_ind++;
                        }
                        i++;
                        }
                    }
                }     
            }
        // }
    }
    {
#ifdef TRACY
        ZoneNamedN(Zone10, "Renderer::PrepareInstanceBuffer::Unmap-anim", true);
#endif

        vkUnmapMemory(Gfx::Device::GetInstance()->GetDevice(),Gfx::DescriptorsBase::GetInstance()->GetAnimationBufferMemory(GetFrameInd()));
    }
    }
#endif
}
void Gfx::Renderer::PrepareCameraBuffer(Keeper::Camera& camera)
{
#ifdef TRACY
    ZoneScopedN("Renderer::PrepareCameraBuffer");
#endif
	glm::mat4 view = glm::lookAt(camera.GetPos(), camera.GetPos() + camera.GetFront(), camera.GetUp());
	glm::mat4 projection = glm::perspective(glm::radians(45.f), float(Gfx::Device::GetInstance()->GetSwapchainSize().x) / float(Gfx::Device::GetInstance()->GetSwapchainSize().y), 1.0f, 1000.0f);
	projection[1][1] *= -1;
    
    glm::mat4 sky_projection = glm::perspective(glm::radians(45.f), float(Gfx::Device::GetInstance()->GetSwapchainSize().x) / float(Gfx::Device::GetInstance()->GetSwapchainSize().y), 0.10f, 1000.0f);
	sky_projection[1][1] *= -1;
    
    if (Core::Scene::GetInstance()->HasGameModules() && Core::Scene::GetInstance()->HasAnimator()) {
	    CamData.global_transform = Core::Scene::GetInstance()->GetGlobalTransform();
    }
    CamData.compute_skinning_size = Core::Scene::GetInstance()->GetSceneVertSize();
ComputeSkinningMaxVertex = CamData.compute_skinning_size;
    CamData.model = glm::mat4(1.0f);
	CamData.proj = projection;
	CamData.view = view;
	CamData.skybox_proj = sky_projection;
	CamData.viewpos = glm::vec4(camera.GetPos(), 1.0);
    //printf("%f|%f|%f\n", camera.GetPos().x, camera.GetPos().y, camera.GetPos().z);
	CamData.mousepos = { Core::WindowManager::GetInstance()->GetMouseBeginPress().x, Core::WindowManager::GetInstance()->GetMouseBeginPress().y, 0, 0};
	CamData.viewport = /*{ Gfx::Device::GetInstance()->GetSwapchainSize().x, Gfx::Device::GetInstance()->GetSwapchainSize().y , 0, 0 };*/{ Core::WindowManager::GetInstance()->GetScreenResolution().x ,Core::WindowManager::GetInstance()->GetScreenResolution().y, 0, 0};
    CamData.time = static_cast<float>(Engine::GetInstance()->GetTimeSinceStart()) / 1000.0;
        
    CamData.global_light_dir = glm::vec4(LightData.GlobalDirection, 1.0);
    CamData.specular = glm::vec4(LightData.Specular, 1.0);
    CamData.diffuse = glm::vec4(LightData.Diffuse, 1.0);
    CamData.ambient = glm::vec4(LightData.Ambient, 1.0);
    
    CamData.hascubemap = HasFrameCubemap && !Core::Scene::GetInstance()->GetParticles();
    CamData.hasshadows = Core::Scene::GetInstance()->GetShadows();
    CamData.cubemapbind = Core::Scene::GetInstance()->GetCubemapBind(GetFrameInd());
    Keeper::GameObjectsMap map = Core::Scene::GetInstance()->GetGameObjects()->GetObjects();
    int i = 0;
    int j = 0;
    for (Keeper::Objects* obj :  map[Keeper::Type::LIGHT]) {
        if (i < MAX_POINT_LIGHT) {
            CamData.point_light_pos[i] = glm::vec4(obj->GetPosition().x, obj->GetPosition().y, obj->GetPosition().z, 0.0f);
            CamData.point_light_color[i] = glm::vec4(obj->GetColor().x, obj->GetColor().y, obj->GetColor().z, 0.0f);
            float constant = 1.0; 
            float linear = 0.7;
            float quadratic = 1.8;
            float light_max = std::fmaxf(std::fmaxf(obj->GetColor().x, obj->GetColor().y), obj->GetColor().z);
            float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0 / 5.0) * light_max))) / (2 * quadratic);
            if (j >= 4) {
                j = 0;
            }
            CamData.point_light_radius[i][j] = radius;
            j++;
           // printf("%d|RADIUS %f, %f, %f, %f\n",i, radius, obj->GetColor().x, obj->GetColor().y, obj->GetColor().z);
            i++;
        }
    }

    CamData.point_light_size = i;
    CamData.global_animation_bind = global_animation_bind[FrameIndex]; 
    glm::vec3 light_pos = glm::vec3(50.0f, 0.0f, -15.0f);
    float near_plane = 1.0f;
    float far_plane = 1000.0f;
   
    glm::mat4 light_view = glm::lookAt(LightData.GlobalDirection, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 light_proj = glm::perspective(glm::radians(45.0f), 1.0f, near_plane, far_plane);
    light_proj[1][1] *= -1;

    CamData.shadow_matrix = light_proj * light_view;
    const size_t buffersize = (sizeof(CameraData));
    BufferHelper::MapMemory(Gfx::DescriptorsBase::GetInstance()->GetCameraUBO(GetFrameInd()), buffersize, 0, &CamData);
}

VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags)
{
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.pNext = nullptr;
    fenceCreateInfo.flags = flags;
    return fenceCreateInfo;
}

VkSemaphoreCreateInfo SemaphoreCreateInfo(VkSemaphoreCreateFlags flags)
{
    VkSemaphoreCreateInfo semCreateInfo = {};
    semCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semCreateInfo.pNext = nullptr;
    semCreateInfo.flags = flags;
    return semCreateInfo;
}

VkCommandBufferBeginInfo CmdBeginInfo(VkCommandBufferUsageFlags flags)
{
    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext = nullptr;

    info.pInheritanceInfo = nullptr;
    info.flags = flags;
    return info;
}

VkSubmitInfo SubmitInfo(VkCommandBuffer* cmd)
{
    VkSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.pNext = nullptr;

    info.waitSemaphoreCount = 0;
    info.pWaitSemaphores = nullptr;
    info.pWaitDstStageMask = nullptr;
    info.commandBufferCount = 1;
    info.pCommandBuffers = cmd;
    info.signalSemaphoreCount = 0;
    info.pSignalSemaphores = nullptr;

    return info;
}

uint32_t Gfx::Renderer::SyncFrame()
{
    Time = Engine::GetInstance()->GetTime();
    
    if (SwapchainIndex >= MAX_FRAMES + 1 || SwapchainIndex < 0) {
        return -1;
    }
	VK_ASSERT(vkWaitForFences(Gfx::Device::GetInstance()->GetDevice(), 1, &Frames[SwapchainIndex].RenderFence, true, 1000000000), "vkWaitForFences failed !");
	uint32_t swapchainimageindex;
    PrevSwapchainIndex = SwapchainIndex;
	VkResult e = vkAcquireNextImageKHR(Gfx::Device::GetInstance()->GetDevice(), Gfx::Device::GetInstance()->GetSwapchain(), 1000000000, Frames[PrevSwapchainIndex].PresentSemaphore, VK_NULL_HANDLE, &swapchainimageindex);
	if (e == VK_ERROR_OUT_OF_DATE_KHR) {
    	OnResize = true;
		return -1;
	}
  
	VK_ASSERT(vkResetFences(Gfx::Device::GetInstance()->GetDevice(), 1, &Frames[swapchainimageindex].RenderFence), "vkResetFences failed !");
	VK_ASSERT(vkResetCommandBuffer(Frames[FrameIndex].MainCommandBuffer, 0), "vkResetCommandBuffer failed!");
    for (int i = 0; i < Thread::MAX_RENDER_THREAD_NUM; i++) {
        VK_ASSERT(vkResetCommandPool(Gfx::Device::GetInstance()->GetDevice(), Frames[FrameIndex].SecondaryCmd[i].Pool, 0), "Failed to reset command pool!");
    }
  	return swapchainimageindex;
}

void Gfx::Renderer::Sync()
{
    SwapchainIndex = 0;
   	VkFenceCreateInfo fencecreateinfo = FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	VkSemaphoreCreateInfo semcreateinfo = SemaphoreCreateInfo(0);

	VkFenceCreateInfo uploadfencecreateinfo = FenceCreateInfo(0);
	VK_ASSERT(vkCreateFence(Gfx::Device::GetInstance()->GetDevice(), &uploadfencecreateinfo, nullptr, &Upload.UploadFence), "failed to create upload fence !");
	Core::Deletor::GetInstance()->Push(Core::Deletor::Type::SYNC, [=, this]() {
		vkDestroyFence(Gfx::Device::GetInstance()->GetDevice(), Upload.UploadFence, nullptr);
	});
	for (int i = 0; i < MAX_FRAMES + 1; i++) {
		VK_ASSERT(vkCreateFence(Gfx::Device::GetInstance()->GetDevice(), &fencecreateinfo, nullptr, &Frames[i].RenderFence), "failed to create fence !");

		VK_ASSERT(vkCreateSemaphore(Gfx::Device::GetInstance()->GetDevice(), &semcreateinfo, nullptr, &Frames[i].PresentSemaphore), "failed to create present semaphore!");
		VK_ASSERT(vkCreateSemaphore(Gfx::Device::GetInstance()->GetDevice(), &semcreateinfo, nullptr, &Frames[i].RenderSemaphore), "failed to create render semaphore!");

		Core::Deletor::GetInstance()->Push(Core::Deletor::Type::SYNC, [=, this]() {
			vkDestroyFence(Gfx::Device::GetInstance()->GetDevice(), Frames[i].RenderFence, nullptr);
			vkDestroySemaphore(Gfx::Device::GetInstance()->GetDevice(), Frames[i].PresentSemaphore, nullptr);
			vkDestroySemaphore(Gfx::Device::GetInstance()->GetDevice(), Frames[i].RenderSemaphore, nullptr);
		});
	}
}

void Gfx::Renderer::CleanResources()
{
	for (auto& list : Textures) {
        vkDestroySampler(Gfx::Device::GetInstance()->GetDevice(), *list.second.GetSampler(), nullptr);
		vkDestroyImageView(Gfx::Device::GetInstance()->GetDevice(), list.second.GetImageView(), nullptr);
        vkDestroyImage(Gfx::Device::GetInstance()->GetDevice(), list.second.GetImage(), nullptr);
	    vkFreeMemory(Gfx::Device::GetInstance()->GetDevice(), list.second.GetDeviceMemory(), nullptr);
    }
    Textures.clear();

    for (auto& list : Cubemaps) {
        vkDestroySampler(Gfx::Device::GetInstance()->GetDevice(), *list.second.GetSampler(), nullptr);
		vkDestroyImageView(Gfx::Device::GetInstance()->GetDevice(), list.second.GetImageView(), nullptr);
        vkDestroyImage(Gfx::Device::GetInstance()->GetDevice(), list.second.GetImage(), nullptr);
	    vkFreeMemory(Gfx::Device::GetInstance()->GetDevice(), list.second.GetDeviceMemory(), nullptr);
    }
    Cubemaps.clear();

    for (auto& it : CubemapsImgui) {
        it.second.Clean();
    }
    CubemapsImgui.clear();


    for (int i = 0; i < Gfx::RT_SIZE; i++) {
        if (RTs[i]) {
            if (RTs[i]->IsSamplerSet()) {
                vkDestroySampler(Gfx::Device::GetInstance()->GetDevice(), *(RTs[i]->GetSampler()), nullptr);
            }
            vkDestroyImageView(Gfx::Device::GetInstance()->GetDevice(), RTs[i]->GetImageView(), nullptr);
		    vkDestroyImage(Gfx::Device::GetInstance()->GetDevice(), RTs[i]->GetImage(), nullptr);
		    vkFreeMemory(Gfx::Device::GetInstance()->GetDevice(), RTs[i]->GetDeviceMemory(), nullptr);
		    delete RTs[i];
        }
    }
}

void Gfx::Renderer::DestroyRenderTarget(Gfx::RenderTarget* rt)
{
    if (rt->IsSamplerSet()) {
	    vkDestroySampler(Gfx::Device::GetInstance()->GetDevice(), *(rt->GetSampler()), nullptr);
    }
    rt->SetSampler(false);
    vkDestroyImageView(Gfx::Device::GetInstance()->GetDevice(), rt->GetImageView(), nullptr);
	vkDestroyImage(Gfx::Device::GetInstance()->GetDevice(), rt->GetImage(), nullptr);
	vkFreeMemory(Gfx::Device::GetInstance()->GetDevice(), rt->GetDeviceMemory(), nullptr);
	delete rt;
}

void Gfx::Renderer::CreateRenderTargets()
{
    if (RTs[Gfx::RT_DEPTH]) {
        DestroyRenderTarget(RTs[Gfx::RT_DEPTH]);
    }
    RTs[Gfx::RT_DEPTH] = new RenderTarget(Gfx::RT_DEPTH);
	RTs[Gfx::RT_DEPTH]->SetFormat(VK_FORMAT_D32_SFLOAT);
	RTs[Gfx::RT_DEPTH]->SetDepth(true);
	RTs[Gfx::RT_DEPTH]->SetDimensions({(int)Gfx::Device::GetInstance()->GetSwapchainExtent().width, (int)Gfx::Device::GetInstance()->GetSwapchainExtent().height});
    RTs[Gfx::RT_DEPTH]->CreateRenderTarget();
    CreateSampler(RTs[Gfx::RT_DEPTH]);

    if (RTs[Gfx::RT_MAIN_DEBUG]) {
        DestroyRenderTarget(RTs[Gfx::RT_MAIN_DEBUG]);
    }
    RTs[Gfx::RT_MAIN_DEBUG] = new RenderTarget(*RTs[Gfx::RT_DEPTH], Gfx::RT_MAIN_DEBUG);
	RTs[Gfx::RT_MAIN_DEBUG]->SetFormat(VK_FORMAT_B8G8R8A8_SRGB);
	RTs[Gfx::RT_MAIN_DEBUG]->SetDepth(false);
	RTs[Gfx::RT_MAIN_DEBUG]->CreateRenderTarget();
    CreateSampler(RTs[Gfx::RT_MAIN_DEBUG]);

    if (RTs[Gfx::RT_MAIN_COLOR]) {
        DestroyRenderTarget(RTs[Gfx::RT_MAIN_COLOR]);
    }
    RTs[Gfx::RT_MAIN_COLOR] = new RenderTarget(*RTs[Gfx::RT_DEPTH], Gfx::RT_MAIN_COLOR);
	RTs[Gfx::RT_MAIN_COLOR]->SetFormat(VK_FORMAT_B8G8R8A8_SRGB);
	RTs[Gfx::RT_MAIN_COLOR]->SetDepth(false);
	RTs[Gfx::RT_MAIN_COLOR]->CreateRenderTarget();


    if (RTs[Gfx::RT_MASK]) {
        DestroyRenderTarget(RTs[Gfx::RT_MASK]);
    }
	RTs[Gfx::RT_MASK] = new RenderTarget(*RTs[Gfx::RT_MAIN_COLOR], Gfx::RT_MASK);
    RTs[Gfx::RT_MASK]->SetFormat(VK_FORMAT_R8_UNORM);
    RTs[Gfx::RT_MASK]->CreateRenderTarget();
    CreateSampler(RTs[Gfx::RT_MASK]);

    if (RTs[Gfx::RT_NORMAL]) {
        DestroyRenderTarget(RTs[Gfx::RT_NORMAL]);
    }
    RTs[Gfx::RT_NORMAL] = new RenderTarget(*RTs[Gfx::RT_MAIN_COLOR], Gfx::RT_NORMAL);
    RTs[Gfx::RT_NORMAL]->SetFormat(VK_FORMAT_R16G16B16A16_SFLOAT);
    RTs[Gfx::RT_NORMAL]->CreateRenderTarget();
    CreateSampler(RTs[Gfx::RT_NORMAL]);

    if (RTs[Gfx::RT_POSITION]) {
        DestroyRenderTarget(RTs[Gfx::RT_POSITION]);
    }
    RTs[Gfx::RT_POSITION] = new RenderTarget(*RTs[Gfx::RT_MAIN_COLOR], Gfx::RT_POSITION);
    RTs[Gfx::RT_POSITION]->SetFormat(VK_FORMAT_R16G16B16A16_SFLOAT);
    RTs[Gfx::RT_POSITION]->CreateRenderTarget();
    CreateSampler(RTs[Gfx::RT_POSITION]);

    if (RTs[Gfx::RT_ALBEDO]) {
        DestroyRenderTarget(RTs[Gfx::RT_ALBEDO]);
    }
    RTs[Gfx::RT_ALBEDO] = new RenderTarget(*RTs[Gfx::RT_MAIN_COLOR], Gfx::RT_ALBEDO);
    RTs[Gfx::RT_ALBEDO]->SetFormat(VK_FORMAT_R16G16B16A16_SFLOAT);
    RTs[Gfx::RT_ALBEDO]->CreateRenderTarget();
    CreateSampler(RTs[Gfx::RT_ALBEDO]);
    
    if (RTs[Gfx::RT_SHADOWS]) {
        DestroyRenderTarget(RTs[Gfx::RT_SHADOWS]);
    }
    RTs[Gfx::RT_SHADOWS] = new RenderTarget(*RTs[Gfx::RT_DEPTH], Gfx::RT_SHADOWS);
    RTs[Gfx::RT_SHADOWS]->SetFormat(VK_FORMAT_D32_SFLOAT);
	RTs[Gfx::RT_SHADOWS]->SetDepth(true);
    RTs[Gfx::RT_SHADOWS]->SetDimensions({1028, 1028});
    RTs[Gfx::RT_SHADOWS]->CreateRenderTarget();
    CreateSampler(RTs[Gfx::RT_SHADOWS]);

    CreateImGuiDescSet();
}

void Gfx::Renderer::CreateImGuiDescSet()
{
    if (Core::ImGuiHelper::GetInstance()->IsInit()) {
        RTs[Gfx::RT_MAIN_DEBUG]->SetImGuiDescriptor();
        RTs[Gfx::RT_MASK]->SetImGuiDescriptor();
        RTs[Gfx::RT_POSITION]->SetImGuiDescriptor();
        RTs[Gfx::RT_NORMAL]->SetImGuiDescriptor();
        RTs[Gfx::RT_ALBEDO]->SetImGuiDescriptor();
        RTs[Gfx::RT_DEPTH]->SetImGuiDescriptor(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL);
        RTs[Gfx::RT_SHADOWS]->SetImGuiDescriptor(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL);
    }
}

std::vector<std::string> Gfx::Renderer::GetRTList()
{
    std::vector<std::string> names;
    names.reserve(Gfx::RT_SIZE);
    for (int i = 0; i < Gfx::RT_SIZE; i++) {
        if (i != Gfx::RT_MAIN_COLOR) {
            names.emplace_back(Gfx::GetValue(static_cast<Gfx::RenderTargetsList>(i)));
        }
    }
    return names;
}

VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32_t graphicsfamily, VkCommandPoolCreateFlags flags) {

	VkCommandPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.pNext = nullptr;

	info.queueFamilyIndex = graphicsfamily;
	info.flags = flags;
	return info;
}

VkCommandBufferAllocateInfo CommandBufferCreateInfo(VkCommandPool pool, uint32_t count, VkCommandBufferLevel level) {

	VkCommandBufferAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.pNext = nullptr;

	info.commandPool = pool;
	info.commandBufferCount = count;
	info.level = level;
	return info;
}

void Gfx::Renderer::CreateCommands()
{
  	vkCmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR) vkGetInstanceProcAddr(Gfx::Vulkan::GetInstance()->GetVkInstance(), "vkCmdBeginRenderingKHR");
	vkCmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR) vkGetInstanceProcAddr(Gfx::Vulkan::GetInstance()->GetVkInstance(), "vkCmdEndRenderingKHR");
#ifdef TRACY
        Tracy.GetPhysicalDeviceCalibrateableTimeDomainsEXT = (PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT)vkGetInstanceProcAddr(Gfx::Vulkan::GetInstance()->GetVkInstance(), "vkGetPhysicalDeviceCalibrateableTimeDomainsEXT");
        Tracy.GetCalibratedTimestampsEXT = (PFN_vkGetCalibratedTimestampsEXT)vkGetInstanceProcAddr(Gfx::Vulkan::GetInstance()->GetVkInstance(), "vkGetCalibratedTimestampsEXT");
#endif

    Gfx::QueueFamilyIndex indices = Gfx::Device::GetInstance()->FindQueueFamilies(Gfx::Device::GetInstance()->GetPhysicalDevice());
    VkCommandPoolCreateInfo poolinfo = CommandPoolCreateInfo(indices.Graphics.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    for (int i = 0; i < MAX_FRAMES; i++) {
       	VK_ASSERT(vkCreateCommandPool(Gfx::Device::GetInstance()->GetDevice(), &poolinfo, nullptr, &Frames[i].CommandPool), "failed to create command pool!");

		VkCommandBufferAllocateInfo cmdinfo = CommandBufferCreateInfo(Frames[i].CommandPool, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

		VK_ASSERT(vkAllocateCommandBuffers(Gfx::Device::GetInstance()->GetDevice(), &cmdinfo, &Frames[i].MainCommandBuffer), "failed to allocate command buffers!");

        // secondary cmd size of thread pool
        Frames[i].SecondaryCmd.resize(Thread::MAX_RENDER_THREAD_NUM);
        for (int j = 0; j < Thread::MAX_RENDER_THREAD_NUM; j++) {
            ASSERT(vkCreateCommandPool(Gfx::Device::GetInstance()->GetDevice(), &poolinfo, nullptr, &Frames[i].SecondaryCmd[j].Pool), "failed to create command pool!");

		    VkCommandBufferAllocateInfo seccmdinfo = CommandBufferCreateInfo(Frames[i].SecondaryCmd[j].Pool, 1, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

		    VK_ASSERT(vkAllocateCommandBuffers(Gfx::Device::GetInstance()->GetDevice(), &seccmdinfo, &Frames[i].SecondaryCmd[j].Cmd), "failed to allocate command buffers!");
            Core::Deletor::GetInstance()->Push(Core::Deletor::Type::CMD, [=, this]() {
			    vkDestroyCommandPool(Gfx::Device::GetInstance()->GetDevice(), Frames[i].SecondaryCmd[j].Pool, nullptr);
		    });
        }
		Core::Deletor::GetInstance()->Push(Core::Deletor::Type::CMD, [=, this]() {
			vkDestroyCommandPool(Gfx::Device::GetInstance()->GetDevice(), Frames[i].CommandPool, nullptr);
		});
	}


	VK_ASSERT(vkCreateCommandPool(Gfx::Device::GetInstance()->GetDevice(), &poolinfo, nullptr, &Upload.CommandPool), "failed to create upload command pool!");
	Core::Deletor::GetInstance()->Push(Core::Deletor::Type::CMD, [=, this]() {
		vkDestroyCommandPool(Gfx::Device::GetInstance()->GetDevice(), Upload.CommandPool, nullptr);
	});
	VkCommandBufferAllocateInfo cmdallocinfo = CommandBufferCreateInfo(Upload.CommandPool, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	VK_ASSERT(vkAllocateCommandBuffers(Gfx::Device::GetInstance()->GetDevice(), &cmdallocinfo, &Upload.CommandBuffer), "failed to allocate upload command buffers!");

#ifdef TRACY
    VK_ASSERT(vkCreateCommandPool(Gfx::Device::GetInstance()->GetDevice(), &poolinfo, nullptr, &Tracy.Pool), "failed to create upload command pool!");
	Core::Deletor::GetInstance()->Push(Core::Deletor::Type::CMD, [=, this]() {
		vkDestroyCommandPool(Gfx::Device::GetInstance()->GetDevice(), Tracy.Pool, nullptr);
	});
	VkCommandBufferAllocateInfo cmdallocinfotracy = CommandBufferCreateInfo(Tracy.Pool, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	VK_ASSERT(vkAllocateCommandBuffers(Gfx::Device::GetInstance()->GetDevice(), &cmdallocinfotracy, &Tracy.Cmd), "failed to allocate upload command buffers!");
#endif

}

void Gfx::Renderer::Submit(std::function<void(VkCommandBuffer cmd)>&& function)
{
	VkCommandBuffer cmd = Upload.CommandBuffer;

	VkCommandBufferBeginInfo cmdBeginInfo = CmdBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_ASSERT(vkBeginCommandBuffer(cmd, &cmdBeginInfo), "failed to begin command buffer!");
	function(cmd);
	VK_ASSERT(vkEndCommandBuffer(cmd), "failed to end command buffer!");

	VkSubmitInfo submitinfo = SubmitInfo(&cmd);
	VK_ASSERT(vkQueueSubmit(Gfx::Device::GetInstance()->GetQueue(GRAPHICS_QUEUE), 1, &submitinfo, Upload.UploadFence), "failed to submit upload queue!");

	vkWaitForFences(Gfx::Device::GetInstance()->GetDevice(), 1, &Upload.UploadFence, true, 9999999999);
	vkResetFences(Gfx::Device::GetInstance()->GetDevice(), 1, &Upload.UploadFence);
	vkResetCommandPool(Gfx::Device::GetInstance()->GetDevice(), Upload.CommandPool, 0);
}
