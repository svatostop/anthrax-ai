module;
#include "aai/gfx/vk/backend/vk_defines.h"
#include "aai/gfx/vk/model/model_types.h"
#include <vulkan/vulkan_core.h>

module aai.gfx.vk.renderer;
import aai.gfx.vk.pipeline;
import std;
void vk::renderer::init(VkInstance inst, const vk::device::handlers& dev, VkDescriptorSet bindless, VkDeviceAddress buffer_addr, VkDeviceAddress instance_addr)
{
    bindless_set = bindless;
    camera_buffer_address = buffer_addr;
    instance_buffer_address = instance_addr;

    vkCmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR) vkGetInstanceProcAddr(inst, "vkCmdBeginRenderingKHR");
	vkCmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR) vkGetInstanceProcAddr(inst, "vkCmdEndRenderingKHR");

    rts.create(dev, window_size);
    rts.fill_refs();
}

void vk::renderer::refresh_state()
{
    state.instance_ind = 0;
    state.check_material = nullptr;
    state.check_mesh = nullptr;
}

void vk::renderer::check_render_state(const rq::data& rq)
{
    state.attachment_rule = rt::helper::rule::LOAD;
    if (state.attachment_ref.id != rq.material_handle->attachment_ref.id) {
        state.attachment_rule = rt::helper::rule::CLEAR;
        state.attachment_ref = rq.material_handle->attachment_ref;
    } 
}

void vk::renderer::block(VkCommandBuffer cmd, const rq::data& rq)
{
    check_render_state(rq);
        
    start_render(cmd, rq.material_handle->attachment_ref);

    if (rq.material_handle->dynamic_viewport) {
        set_dynamic_viewport(cmd);
    }

    if (rq.mesh_handle) {
        check_bindings(nullptr, rq.mesh_handle);
        if (state.bind_mesh) {
            VkDeviceSize offsets[1] = { 0 };
            VkBuffer vert_buf = rq.mesh_handle->get_vertex_buffer();
            vkCmdBindVertexBuffers(cmd, 0, 1, &vert_buf, offsets);
            vkCmdBindIndexBuffer(cmd, rq.mesh_handle->get_index_buffer(), 0, VK_INDEX_TYPE_UINT16);
        }
        for (const model::types::node* n : rq.mesh_handle->get_nodes())    
            draw(cmd, rq, n);
    }
    else
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
        info.loadOp = ((loadop & rt::helper::rule::LOAD) == rt::helper::rule::LOAD) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
        info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        info.clearValue = clearvalue;
	}
    else {
		clearvalue.depthStencil = {1.0f, 0};

		info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		info.imageView = imageview;
		info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		info.resolveMode = VK_RESOLVE_MODE_NONE;
		info.loadOp = ((loadop & rt::helper::rule::LOAD) == rt::helper::rule::LOAD) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
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

    VkRect2D renderarea = VkRect2D{VkOffset2D{}, { static_cast<uint32_t>(window_size.x) , static_cast<uint32_t>(window_size.y) } };
	VkRenderingInfoKHR renderinfo {
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
		.renderArea = renderarea,
		.layerCount = 1,
	};
    
    VkRenderingAttachmentInfoKHR depth_info{};
    if (attachment_ref.depth_count > 0) {
        depth_info = get_attachment_info(rts.get_rt(attachment_ref.depth_types.v)->get_image_view(), false, state.attachment_rule);
        renderinfo.pDepthAttachment = &depth_info;
    }
    std::vector<VkRenderingAttachmentInfoKHR> colors;
    if (attachment_ref.color_count > 0) {
        colors.reserve(attachment_ref.color_count);
        for (const rt::base::type& t : attachment_ref.color_types) {
            colors.push_back(get_attachment_info(rts.get_rt(t.v)->get_image_view(), true, state.attachment_rule));
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
void vk::renderer::draw(VkCommandBuffer cmd, const rq::data& rq, const model::types::node* n)
{
    if (n->mesh.primitives.size() > 0) {
        check_bindings(rq.material_handle, rq.mesh_handle);
        if (state.bind_pipeline) {
            if (rq.texture_id > 0) {
                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, rq.material_handle->pipeline_layout , 0, 1, &bindless_set, 0, nullptr);
            }
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, rq.material_handle->pipeline);
        }
    
        vk::pipeline::push_range constants;
        constants.camera_gpu_address = camera_buffer_address;
        constants.instance_gpu_address = instance_buffer_address;
        constants.texture_id = rq.texture_id;
        constants.skin_gpu_address = 0;
        if (n->skin_id > -1) {
            constants.skin_gpu_address = rq.mesh_handle->get_skin_gpu_address(n->skin_id); 
        }
        vkCmdPushConstants(cmd, rq.material_handle->pipeline_layout , VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(vk::pipeline::push_range), &constants);

        for (const model::types::primitive& prim : n->mesh.primitives) {
            if (prim.index_count <= 0)
                continue;
            vkCmdDrawIndexed(cmd, prim.index_count, 1, prim.first_index, 0, state.instance_ind);
        }
    }
    
    for (model::types::node* child : n->children) {
        draw(cmd, rq, child);
    }
}

void vk::renderer::draw(VkCommandBuffer cmd, const rq::data& rq)
{
    check_bindings(rq.material_handle, nullptr);
    if (state.bind_pipeline) {
        if (rq.texture_id > 0) {
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, rq.material_handle->pipeline_layout , 0, 1, &bindless_set, 0, nullptr);
        }
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, rq.material_handle->pipeline);
    }
    
	vk::pipeline::push_range constants;
    constants.camera_gpu_address = camera_buffer_address;
    constants.texture_id = rq.texture_id;
	vkCmdPushConstants(cmd, rq.material_handle->pipeline_layout , VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(vk::pipeline::push_range), &constants);
    
    vkCmdDraw(cmd, 6, 1, 0, 0);
    
}

void vk::renderer::check_bindings(std::shared_ptr<mat::data> material, std::shared_ptr<model::base> mesh)
{
	state.bind_pipeline = state.check_material != material;
    state.bind_mesh = state.check_mesh != mesh;
    state.check_material = state.bind_pipeline  ? material : state.check_material;
	state.check_mesh = state.bind_mesh ? mesh : state.check_mesh;
}

void vk::renderer::set_dynamic_viewport(VkCommandBuffer cmd)
{
    VkViewport viewport{};
    viewport.x = 0;
	viewport.y = 0;
	viewport.width = (float)window_size.x;
	viewport.height = (float)window_size.y;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    
    VkRect2D scissor{};
    scissor.offset.x = 0;
    scissor.offset.y = 0; 
	scissor.extent.width = static_cast<uint32_t>(window_size.x);
    scissor.extent.height = static_cast<uint32_t>(window_size.y );
    vkCmdSetScissor(cmd, 0, 1, &scissor);
}

