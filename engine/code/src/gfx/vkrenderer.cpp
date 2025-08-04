#include "anthraxAI/gfx/vkrenderer.h"
#include "anthraxAI/core/scene.h"
#include "anthraxAI/gamemodules/modules.h"
#include "anthraxAI/gameobjects/gameobjects.h"
#include "anthraxAI/gfx/bufferhelper.h"
#include "anthraxAI/gfx/renderhelpers.h"
#include "anthraxAI/gfx/vkdevice.h"
#include "anthraxAI/gfx/vkbase.h"
#include "anthraxAI/gfx/vkdescriptors.h"
#include "anthraxAI/core/windowmanager.h"
#include "anthraxAI/gameobjects/objects/camera.h"
#include "anthraxAI/gfx/vkmesh.h"
#include "anthraxAI/gfx/vkrendertarget.h"
#include "anthraxAI/utils/debug.h"
#include "anthraxAI/utils/defines.h"
#include "anthraxAI/utils/mathdefines.h"
#include "anthraxAI/utils/thread.h"
#include "anthraxAI/gfx/vkdefines.h"
#include "glm/detail/qualifier.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <random>

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
    constants.selected = 0;
    constants.boneID = -1;
    if (object.HasStorage) {
        constants.storagebind = object.StorageBind[GetFrameInd()];
        constants.instancebind = object.InstanceBind[GetFrameInd()];
        constants.objectID = object.ID;
        constants.selected = (object.IsSelected || object.ID == Core::Scene::GetInstance()->GetSelectedID()) ? 1 : 0;
        constants.boneID = Utils::Debug::GetInstance()->BoneID;
    }
	vkCmdPushConstants(Cmd.GetCmd(), object.Material->PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Gfx::MeshPushConstants), &constants);
    if (object.IsCompute) {
        VkDeviceSize offset = {0};
        VkBuffer buffer = Gfx::DescriptorsBase::GetInstance()->GetComputeBuffer(GetFrameInd());
        vkCmdBindVertexBuffers(Cmd.GetCmd(), 0, 1, &buffer, &offset);
	    vkCmdDraw(Cmd.GetCmd(), u_int32_t(NUM_PARTICLES / sizeof(glm::vec2) ), 1, 0, 0);
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
	bool bindpipe, bindindex = false;
	CheckTmpBindings(mesh, object.Material, &bindpipe, &bindindex);

	if (bindpipe) {
	    vkCmdBindDescriptorSets(Cmd.GetCmd(), VK_PIPELINE_BIND_POINT_GRAPHICS, object.Material->PipelineLayout, 0, 1, Gfx::DescriptorsBase::GetInstance()->GetBindlessSet(GetFrameInd()), 0, nullptr);
		vkCmdBindPipeline(Cmd.GetCmd(), VK_PIPELINE_BIND_POINT_GRAPHICS, object.Material->Pipeline);
    }

	Gfx::MeshPushConstants constants;
	constants.texturebind = object.TextureBind[GetFrameInd()];
	constants.bufferbind = object.BufferBind[GetFrameInd()];
    constants.selected = 0;
    constants.boneID = -1;
    if (object.HasStorage) {
        constants.storagebind = object.StorageBind[GetFrameInd()];
        constants.instancebind = object.InstanceBind[GetFrameInd()];
        constants.objectID = object.ID;
        constants.selected = (object.IsSelected || object.ID == Core::Scene::GetInstance()->GetSelectedID()) ? 1 : 0;
        if (Utils::Debug::GetInstance()->Bones) {
            constants.boneID = Utils::Debug::GetInstance()->BoneID;
        }
        constants.gizmo = object.GizmoType;
    }
	vkCmdPushConstants(Cmd.GetCmd(), object.Material->PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Gfx::MeshPushConstants), &constants);

	if (bindindex) {
		VkDeviceSize offset = {0};
		vkCmdBindVertexBuffers(Cmd.GetCmd(), 0, 1, &mesh->VertexBuffer.Buffer, &offset);
		vkCmdBindIndexBuffer(Cmd.GetCmd(), mesh->IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT16);
	}
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

void Gfx::Renderer::Compute(Gfx::RenderObject& object)
{
    Cmd.MemoryBarrier(0, 0, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

	bool bindpipe, bindindex = false;
    Gfx::Material* mat = Gfx::Pipeline::GetInstance()->GetMaterial("particles");
    CheckTmpBindings(nullptr, mat, &bindpipe, &bindindex);

	if (bindpipe) {
	    vkCmdBindDescriptorSets(Cmd.GetCmd(), VK_PIPELINE_BIND_POINT_COMPUTE, mat->PipelineLayout, 0, 1, Gfx::DescriptorsBase::GetInstance()->GetBindlessSet(GetFrameInd()), 0, nullptr);
		vkCmdBindPipeline(Cmd.GetCmd(), VK_PIPELINE_BIND_POINT_COMPUTE, mat->Pipeline);
    }

	Gfx::MeshPushConstants constants;
    constants.selected = 0;
    constants.boneID = -1;
    constants.storagebind = object.StorageBind[GetFrameInd()];
	vkCmdPushConstants(Cmd.GetCmd(), mat->PipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(Gfx::MeshPushConstants), &constants);

    vkCmdDispatch(Cmd.GetCmd(), u_int32_t(NUM_PARTICLES / sizeof(glm::vec2) ) / NUM_PARTICLES_PER_WORKGROUP, 1, 1);

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
        if ((rules & Gfx::ATTACHMENT_RULE_LOAD) == Gfx::ATTACHMENT_RULE_LOAD) {
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
   // GetRT(Gfx::RT_MASK)->MemoryBarrier(Cmd.GetCmd(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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

/*void Gfx::Renderer::GetTransforms(InstanceData* datas, Gfx::RenderObject obj, int i)*/
/*{*/
/*    std::vector<glm::mat4> bonevec = {}; */
/*                bonevec = Core::Scene::GetInstance()->UpdateAnimation(obj);*/
/**/
/*                for(int k = 0; (k < obj.Model->Bones.FinTransform.size() ); k++) {*/
/*                    datas[i].bonesmatrices[k] = obj.Model->Bones.FinTransform[k];//vec[i]*/
/*                }*/
/**/
/*}*/

void Gfx::Renderer::PrepareInstanceBuffer()
{
    const size_t buffersize = sizeof(InstanceData) * MAX_INSTANCES ;
    void* instancedata;
    vkMapMemory(Gfx::Device::GetInstance()->GetDevice(),Gfx::DescriptorsBase::GetInstance()->GetInstanceBufferMemory(GetFrameInd()), 0, buffersize, 0, (void**)&instancedata);

    Modules::ScenesMap map =  Core::Scene::GetInstance()->GetScenes();
    Modules::Module& modulegizmo = map["gizmo"];
    Modules::Module& module = map[Core::Scene::GetInstance()->GetCurrentScene()];
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

    /*int i = 0;*/
    /*for (Gfx::RenderObject& obj : module.GetRenderQueue()) {*/
    /**/
    /*    if (!obj.Model || !obj.IsVisible) continue;*/
    /*    for (int j = 0; j < obj.Model->Meshes.size(); j++ ) {*/
    /*     i++;*/
    /*     }*/
    /*} */
    /*for (Gfx::RenderObject& obj : modulegizmo.GetRenderQueue()) {*/
    /*     if (!obj.Model || !obj.IsVisible) continue;*/
    /*        for (int j = 0; j < obj.Model->Meshes.size(); j++ ) {*/
    /*     i++;*/
    /*     }*/
    /*}*/


    /*for (uint32_t in : instance_inds) {*/
    /**/
    /*    printf("INSTANCE PER thread === %d\n", in);*/
    /*}*/

    /*for (uint32_t obj_num = first_obj_size; obj_num < obj_size; obj_num++) {*/
    /*    Gfx::RenderObject& obj = module.GetRenderQueue()[obj_num];*/
    /*    fin_inst_ind2 += obj.Model->Meshes.size();*/
    /**/
    /*}*/
    first_obj_size = 0;
    sec_obj_size = 0;
    uint32_t inst = 0;
    /*for (uint32_t thread_id = 0; thread_id < Thread::MAX_THREAD_NUM; thread_id++) {*/
    /*    sec_obj_size += num_obj_per_thread[thread_id];*/
    /**/
    /*           //printf(" first obj %d === sec obj %d\n", first_obj_size, sec_obj_size);*/
    /*    Thread::Pool::GetInstance()->PushByID(thread_id, { Thread::Task::Name::RENDER, Thread::Task::Type::EXECUTE,*/
    /*    {}, [this,thread_id, &instancedata,&modulegizmo,  &module, inst, first_obj_size, sec_obj_size]() {*/

    InstanceData* datas = (InstanceData*)instancedata;
    /*for (int i = 0; i < InstanceCount; i++) {*/
    /*    datas[i].rendermatrix = glm::mat4(1.0f); */
    /*}*/
    int i = 0;
    //for (auto& it : Core::Scene::GetInstance()->GetScenes()) {

    bool hasanim = false;
    for (auto& it : module.GetRenderQueueMap()) {
    for (Gfx::RenderObject& obj : it.second) {
        /*printf(*/
        /*    "VISIBLE: %d|%s\n", obj.IsVisible, obj.TextureName.c_str()*/
        /*);*/
        if (!obj.Model[GetFrameInd()]) {
            i++;
            continue;
        }
        hasanim = Core::Scene::GetInstance()->HasAnimation(obj.ID);
        for (int j = 0; j < obj.Model[GetFrameInd()]->Meshes.size(); j++ ) {

            if (hasanim) {
               // printf("IIIIII ======================================== %d\n", i);
                for (int k = 0; k < obj.Model[GetFrameInd()]->Bones.Info.size(); k++) {
                    datas[i].bonesmatrices[k] = obj.Model[GetFrameInd()]->Bones.Info[k].FinTransform;
                }
                    //vec[i]
                /*std::thread upd(&Gfx::Renderer::GetTransforms, this, datas, obj, i);*/
                /*upd.join();*/
                            }
            datas[i].hasanimation = hasanim ? 1 : 0;
            datas[i].rendermatrix =glm::translate(glm::mat4(1.0f), glm::vec3(obj.Position.x, obj.Position.y, obj.Position.z));// * CamData.view *  ;
            //glm::mat4(1.0f);
            i++;
        }
    }
    }
    for (Gfx::RenderObject& obj : modulegizmo.GetRenderQueue(Modules::RQ_GENERAL)) {
         if (!obj.Model[GetFrameInd()] || !obj.IsVisible) continue;
        for (int j = 0; j < obj.Model[GetFrameInd()]->Meshes.size(); j++ ) {
            float dist = glm::distance(glm::vec3(CamData.viewpos.x, CamData.viewpos.y, CamData.viewpos.z), glm::vec3(obj.Position.x, obj.Position.y, obj.Position.z) )* 0.05;
            if (dist <= 0.5) {
                dist = 0.5;
            }


            glm::vec3 distfin = glm::vec3(dist);


            datas[i].rendermatrix = glm::translate(glm::mat4(1.0f), glm::vec3(obj.Position.x, obj.Position.y, obj.Position.z));
            datas[i].rendermatrix = glm::scale(datas[i].rendermatrix, distfin);
            i++;
        }

    }


    /*    }, 0,  nullptr, nullptr, nullptr});*/
    /**/
    /*    first_obj_size = sec_obj_size;*/
    /*}            */
    /**/
    /*    Thread::Pool::GetInstance()->Wait();*/

/*
    InstanceData* datas = (InstanceData*)instancedata;
       int i = 0;
    //for (auto& it : Core::Scene::GetInstance()->GetScenes()) {

    for (Gfx::RenderObject& obj : map[Core::Scene::GetInstance()->GetCurrentScene()].GetRenderQueue()) {
        if (!obj.Model || !obj.IsVisible) continue;
        hasanim = Core::Scene::GetInstance()->HasAnimation(obj.ID);
        for (int j = 0; j < obj.Model->Meshes.size(); j++ ) {

            if (hasanim) {
               // printf("IIIIII ======================================== %d\n", i);
                for (int k = 0; k < obj.Model->Bones.Info.size(); k++) {
                    datas[i].bonesmatrices[k] = obj.Model->Bones.Info[k].FinTransform;
                }
                    //vec[i]
                                         }
            datas[i].hasanimation = hasanim ? 1 : 0;
            datas[i].rendermatrix =glm::translate(glm::mat4(1.0f), glm::vec3(obj.Position.x, obj.Position.y, obj.Position.z));// * CamData.view *  ;
            //glm::mat4(1.0f);
            i++;
        }
    }
    for (Gfx::RenderObject& obj : modulegizmo.GetRenderQueue()) {
         if (!obj.Model || !obj.IsVisible) continue;
        for (int j = 0; j < obj.Model->Meshes.size(); j++ ) {
            float dist = glm::distance(glm::vec3(CamData.viewpos.x, CamData.viewpos.y, CamData.viewpos.z), glm::vec3(obj.Position.x, obj.Position.y, obj.Position.z) )* 0.05;
            if (dist <= 0.5) {
                dist = 0.5;
            }


            glm::vec3 distfin = glm::vec3(dist);


            datas[i].rendermatrix = glm::translate(glm::mat4(1.0f), glm::vec3(obj.Position.x, obj.Position.y, obj.Position.z));
            datas[i].rendermatrix = glm::scale(datas[i].rendermatrix, distfin);
            i++;
        }

    }


    /*    }, 0,  nullptr, nullptr, nullptr});*/
    /**/
    /*    first_obj_size = sec_obj_size;*/
    /*}            */
    /**/
    /*    Thread::Pool::GetInstance()->Wait();*/

/*
    InstanceData* datas = (InstanceData*)instancedata;
       int i = 0;
    //for (auto& it : Core::Scene::GetInstance()->GetScenes()) {

    for (Gfx::RenderObject& obj : map[Core::Scene::GetInstance()->GetCurrentScene()].GetRenderQueue()) {
        if (!obj.Model || !obj.IsVisible) continue;
        hasanim = Core::Scene::GetInstance()->HasAnimation(obj.ID);
        for (int j = 0; j < obj.Model->Meshes.size(); j++ ) {

            if (hasanim) {
               // printf("IIIIII ======================================== %d\n", i);
                for (int k = 0; k < obj.Model->Bones.Info.size(); k++) {
                    datas[i].bonesmatrices[k] = obj.Model->Bones.Info[k].FinTransform;
                }
                    //vec[i]
                                         }
            datas[i].hasanimation = hasanim ? 1 : 0;
            datas[i].rendermatrix =glm::translate(glm::mat4(1.0f), glm::vec3(obj.Position.x, obj.Position.y, obj.Position.z));// * CamData.view *  ;
            //glm::mat4(1.0f);
            i++;
        }
    }
    for (Gfx::RenderObject& obj : map["gizmo"].GetRenderQueue()) {
         if (!obj.Model || !obj.IsVisible) continue;
        for (int j = 0; j < obj.Model->Meshes.size(); j++ ) {
            float dist = glm::distance(glm::vec3(CamData.viewpos.x, CamData.viewpos.y, CamData.viewpos.z), glm::vec3(obj.Position.x, obj.Position.y, obj.Position.z) )* 0.05;
            if (dist <= 0.5) {
                dist = 0.5;
            }


            glm::vec3 distfin = glm::vec3(dist);


            datas[i].rendermatrix = glm::translate(glm::mat4(1.0f), glm::vec3(obj.Position.x, obj.Position.y, obj.Position.z));
            datas[i].rendermatrix = glm::scale(datas[i].rendermatrix, distfin);
            i++;
        }

    }*/

    InstanceCount = i;
    InstanceIndex = 0;
    //}
    vkUnmapMemory(Gfx::Device::GetInstance()->GetDevice(),Gfx::DescriptorsBase::GetInstance()->GetInstanceBufferMemory(GetFrameInd()));

}
void Gfx::Renderer::PrepareCameraBuffer(Keeper::Camera& camera)
{
	glm::mat4 view = glm::lookAt(camera.GetPos(), camera.GetPos() + camera.GetFront(), camera.GetUp());
	glm::mat4 projection = glm::perspective(glm::radians(45.f), float(Gfx::Device::GetInstance()->GetSwapchainSize().x) / float(Gfx::Device::GetInstance()->GetSwapchainSize().y), 1.0f, 1000.0f);
	projection[1][1] *= -1;

	CamData.model = glm::mat4(1.0f);
	CamData.proj = projection;
	CamData.view = view;
	CamData.viewproj = projection * view;
	CamData.viewpos = glm::vec4(camera.GetPos(), 1.0);
    //printf("%f|%f|%f\n", camera.GetPos().x, camera.GetPos().y, camera.GetPos().z);
	CamData.mousepos = { Core::WindowManager::GetInstance()->GetMouseBeginPress().x, Core::WindowManager::GetInstance()->GetMouseBeginPress().y, 0, 0};
	CamData.viewport = /*{ Gfx::Device::GetInstance()->GetSwapchainSize().x, Gfx::Device::GetInstance()->GetSwapchainSize().y , 0, 0 };*/{ Core::WindowManager::GetInstance()->GetScreenResolution().x ,Core::WindowManager::GetInstance()->GetScreenResolution().y, 0, 0};
    CamData.time = static_cast<float>(Engine::GetInstance()->GetTimeSinceStart()) / 1000.0;
        
    CamData.global_light_dir = glm::vec4(LightData.GlobalDirection, 1.0);
    CamData.specular = glm::vec4(LightData.Specular, 1.0);
    CamData.diffuse = glm::vec4(LightData.Diffuse, 1.0);
    CamData.ambient = glm::vec4(LightData.Ambient, 1.0);
    
    CamData.hascubemap = HasFrameCubemap;
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
