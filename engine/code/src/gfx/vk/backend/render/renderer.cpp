#include "aai/gfx/vk/backend/vk_defines.h"
#include <vulkan/vulkan_core.h>

import aai.gfx.vk.renderer;
import aai.gfx.vk.pipeline;
import std;
void vk::renderer::init(VkInstance inst, const vk::device::handlers& dev, VkDescriptorSet bindless)
{
    bindless_set = bindless;

    vkCmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR) vkGetInstanceProcAddr(inst, "vkCmdBeginRenderingKHR");
	vkCmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR) vkGetInstanceProcAddr(inst, "vkCmdEndRenderingKHR");

    rts.create(dev);
    rts.fill_refs();
}

void vk::renderer::block(VkCommandBuffer cmd, const rq::data& rq)
{
    start_render(cmd, rq.material_handle->attachment_ref);
    draw(cmd, rq);
    end_render(cmd);
}

VkRenderingAttachmentInfoKHR get_attachment_info(VkImageView imageview, bool iscolor, rt::helper::rule loadop)
{
    VkRenderingAttachmentInfoKHR info = {};
	VkClearValue clearvalue;
    if (iscolor) {
		clearvalue.color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
        info.pNext = nullptr;
        info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        info.imageView = imageview;
        info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        info.resolveMode = VK_RESOLVE_MODE_NONE;
        info.loadOp = ((loadop & rt::helper::rule::ATTACHMENT_RULE_LOAD) == rt::helper::rule::ATTACHMENT_RULE_LOAD) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
        info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        info.clearValue = clearvalue;
	}
    else {
		clearvalue.depthStencil = {1.0f, 0};

		info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		info.imageView = imageview;
		info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		info.resolveMode = VK_RESOLVE_MODE_NONE;
		info.loadOp = ((loadop & rt::helper::rule::ATTACHMENT_RULE_LOAD) == rt::helper::rule::ATTACHMENT_RULE_LOAD) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
		info.storeOp = VK_ATTACHMENT_STORE_OP_STORE ;
		info.clearValue = clearvalue;
	}
	return info;
}

void vk::renderer::start_render(VkCommandBuffer cmd, const rt::base::ref& attachment_ref)
{
    VkImageSubresourceRange range{};
    range.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	range.baseMipLevel   = 0;
	range.levelCount     = VK_REMAINING_MIP_LEVELS;
	range.baseArrayLayer = 0;
	range.layerCount     = VK_REMAINING_ARRAY_LAYERS;

	VkImageSubresourceRange depthrange{range};
	depthrange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	
    VkRect2D renderarea = VkRect2D{VkOffset2D{}, { 800, 600 } };
	VkRenderingInfoKHR renderinfo {
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
		.renderArea = renderarea,
		.layerCount = 1,
	};
    
    VkRenderingAttachmentInfoKHR depth_info{};
    if (attachment_ref.depth_count > 0) {
        depth_info = get_attachment_info(rts.get_rt(attachment_ref.depth_types.v)->get_image_view(), false, rt::helper::rule::ATTACHMENT_RULE_CLEAR);
        renderinfo.pDepthAttachment = &depth_info;
    }
    std::vector<VkRenderingAttachmentInfoKHR> colors;
    if (attachment_ref.color_count > 0) {
        colors.reserve(attachment_ref.color_count);
        for (const rt::base::type& t : attachment_ref.color_types) {
            colors.push_back(get_attachment_info(rts.get_rt(t.v)->get_image_view(), true, rt::helper::rule::ATTACHMENT_RULE_CLEAR));
        }
        renderinfo.colorAttachmentCount = colors.size();
        renderinfo.pColorAttachments = (colors.data());
    }
    vkCmdBeginRenderingKHR(cmd, &renderinfo);
}
void vk::renderer::end_render(VkCommandBuffer cmd)
{
    vkCmdEndRenderingKHR(cmd);
}
void vk::renderer::draw(VkCommandBuffer cmd, const rq::data& rq)
{

    // bool bindpipe, bindindex = false;
	// CheckTmpBindings(object.Mesh, object.Material, &bindpipe, &bindindex);

    //if (bindpipe) 
    {
        if (rq.texture_id > 0) {
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, rq.material_handle->pipeline_layout , 0, 1, &bindless_set, 0, nullptr);
        }
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, rq.material_handle->pipeline);
    }

	vk::pipeline::push_range constants;
    constants.texture_id = rq.texture_id;
	vkCmdPushConstants(cmd, rq.material_handle->pipeline_layout , VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(vk::pipeline::push_range), &constants);
	
    vkCmdDraw(cmd, 6, 1, 0, 0);
    
    //Utils::Debug::GetInstance()->DebugDrawCall();
}

