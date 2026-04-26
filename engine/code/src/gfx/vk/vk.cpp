#include "aai/io/win_defines.h"
#include "aai/gfx/vk/backend/vk_defines.h"
#include <cstdint>
#include <string.h>
#include <vulkan/vulkan_core.h>

import aai.gfx.vk;
import aai.gfx.vk.buffer;
import aai.gfx.vk.device.helper;
import aai.gfx.vk.frames;
import aai.gfx.vk.rt;
import aai.gfx.vk.loader.texture;
import aai.gfx.attachments;
import aai.utils;
import std;

void vk::base::init(bool validate, Display* di, Window w)
{
    inst.init(validate);
#ifdef AAI_LINUX
    dev.init_linux_surface(inst.get_instance(), di, w);
#else 
    dev.init_windows_surface();
#endif
    dev.init(validate, inst.get_layers());

    frame.init(dev.get_device(), dev.get_graphics_index());

    gpu_mem.init(dev.get_devices());
    pipe.set_layout(gpu_mem.get_bindless_layout());
    rt::attachments::create(dev.get_devices());
    rt::attachments::fill();

    submit([&](VkCommandBuffer cmd) {
        rt::attachments::get_rt(rt::attachments::val::MAIN_COLOR)->memory_barrier(cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    });

  	vkCmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR) vkGetInstanceProcAddr(inst.get_instance(), "vkCmdBeginRenderingKHR");
	vkCmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR) vkGetInstanceProcAddr(inst.get_instance(), "vkCmdEndRenderingKHR");
}

bool vk::base::begin_frame()
{
    frame.sync_frames(dev.get_device(), dev.get_swapchain());
    return frame.is_swapchain_index_valid();
}

void vk::base::end_frame()
{
    frame.prepare_for_present(
        rt::attachments::get_rt(rt::attachments::val::MAIN_COLOR)->get_image(),
        rt::attachments::get_rt(rt::attachments::val::MAIN_COLOR)->get_size(),
        dev.get_swapchain_image(frame.get_swapchain_index()),
        { dev.get_swapchain_size().width ,  dev.get_swapchain_size().height }
    );
    frame.submit_and_present(dev.get_queue(vk::queues::type::GRAPHICS), dev.get_swapchain());
}

void vk::base::render()
{
    render_block();
}

void vk::base::render_block()
{
    start_render(rt::attachments::get_rt(rt::attachments::val::MAIN_COLOR));
    draw();
    end_render();
}

VkRenderingAttachmentInfoKHR get_attachment_info(VkImageView imageview, bool iscolor, rt::attachments::rule loadop)
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
        info.loadOp = ((loadop & rt::attachments::rule::ATTACHMENT_RULE_LOAD) == rt::attachments::rule::ATTACHMENT_RULE_LOAD) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
        info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        info.clearValue = clearvalue;
	}
    else {
		clearvalue.depthStencil = {1.0f, 0};

		info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		info.imageView = imageview;
		info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		info.resolveMode = VK_RESOLVE_MODE_NONE;
		info.loadOp = ((loadop & rt::attachments::rule::ATTACHMENT_RULE_LOAD) == rt::attachments::rule::ATTACHMENT_RULE_LOAD) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
		info.storeOp = VK_ATTACHMENT_STORE_OP_STORE ;
		info.clearValue = clearvalue;
	}
	return info;
}

void vk::base::start_render(rt::render_target* target)
{
    VkImageSubresourceRange range{};
    range.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	range.baseMipLevel   = 0;
	range.levelCount     = VK_REMAINING_MIP_LEVELS;
	range.baseArrayLayer = 0;
	range.layerCount     = VK_REMAINING_ARRAY_LAYERS;

	VkImageSubresourceRange depthrange{range};
	depthrange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        
    VkRenderingAttachmentInfoKHR depth_info{};
	
    VkRect2D renderarea = VkRect2D{VkOffset2D{}, { 800, 600 } };
	VkRenderingInfoKHR renderinfo {
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
		.renderArea = renderarea,
		.layerCount = 1,
	};
    
    //for ()
    //if if_depth()
    //    renderinfo.pDepthAttachment = &depthinfo;
    std::vector<VkRenderingAttachmentInfoKHR> colors;
    colors.reserve(1);
    colors.push_back(get_attachment_info(target->get_image_view(), true, rt::attachments::rule::ATTACHMENT_RULE_CLEAR));
    if (!colors.empty()) {
        renderinfo.colorAttachmentCount = colors.size();
	    renderinfo.pColorAttachments = (colors.data());
    }
    
    vkCmdBeginRenderingKHR(frame.get_cmd(), &renderinfo);
}
void vk::base::end_render()
{
    vkCmdEndRenderingKHR(frame.get_cmd());
}
void vk::base::draw()
{
    set_debug_render_pass_name(frame.get_cmd(), "test");

    // bool bindpipe, bindindex = false;
	// CheckTmpBindings(object.Mesh, object.Material, &bindpipe, &bindindex);

    //if (bindpipe) 
    {
        VkDescriptorSet d_set = gpu_mem.get_bindless_set();
        vkCmdBindDescriptorSets(frame.get_cmd(), VK_PIPELINE_BIND_POINT_GRAPHICS, rq.material_handle->pipeline_layout , 0, 1, &d_set, 0, nullptr);
		vkCmdBindPipeline(frame.get_cmd(), VK_PIPELINE_BIND_POINT_GRAPHICS, rq.material_handle->pipeline);
    }

	vk::pipeline::push_range constants;
	vkCmdPushConstants(frame.get_cmd(), rq.material_handle->pipeline_layout , VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(vk::pipeline::push_range), &constants);
	
    vkCmdDraw(frame.get_cmd(), 6, 1, 0, 0);
    
    //Utils::Debug::GetInstance()->DebugDrawCall();
    unset_debug_render_pass_name(frame.get_cmd());
}

void vk::base::submit(std::function<void(VkCommandBuffer cmd)>&& func)
{
	VkCommandBuffer cmd = frame.get_upload_cmd();
	VkCommandBufferBeginInfo cmdBeginInfo = frame.cmd_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    utils::VK_ASSERT(vkBeginCommandBuffer(cmd, &cmdBeginInfo), "failed to begin command buffer!");
	func(cmd);
    utils::VK_ASSERT(vkEndCommandBuffer(cmd), "failed to end command buffer!");

	VkSubmitInfo submitinfo = frame.submit_info(&cmd);
    utils::VK_ASSERT(vkQueueSubmit(dev.get_queue(vk::queues::type::GRAPHICS), 1, &submitinfo, *(frame.get_upload_fence())), "failed to submit upload queue!");

	vkWaitForFences(dev.get_device(), 1, frame.get_upload_fence(), true, 9999999999);
	vkResetFences(dev.get_device(), 1, frame.get_upload_fence());
	vkResetCommandPool(dev.get_device(), frame.get_upload_cmd_pool(), 0);
}

void vk::base::create_texture(const char* path)
{
    int width, height, channels;
    void* pixels = nullptr;
    loader::texture::load_stbi(path, pixels, width, height, channels);
    VkDeviceSize imagesize = width * height * 4;

    utils::ASSERT(!pixels, "failed to load texture image!");

    vk::buffer::handlers stagingbuffer;
    vk::buffer::allocate(stagingbuffer, dev.get_devices(), imagesize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vk::buffer::map_memory(stagingbuffer, dev.get_device(), imagesize, 0, pixels);
    
    loader::texture::unload_stbi(pixels);

    rt::render_target texture(path);
    texture.set_format(VK_FORMAT_R8G8B8A8_SRGB);
    texture.set_dimensions({ width, height });
    texture.set_sampler(true);
    texture.create(dev.get_devices());
    
    set_debug_name(path, reinterpret_cast<uint64_t>(texture.get_device_memory()), VK_OBJECT_TYPE_DEVICE_MEMORY);
    set_debug_name(path, reinterpret_cast<uint64_t>(texture.get_image()), VK_OBJECT_TYPE_IMAGE);
    
    submit([&](VkCommandBuffer cmd) {
        texture.memory_barrier(cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    });
    submit([&](VkCommandBuffer cmd) {
        texture.copy(cmd, stagingbuffer.buffer, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    });
    submit([&](VkCommandBuffer cmd) {
        texture.memory_barrier(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    });

    vkDestroyBuffer(dev.get_device(), stagingbuffer.buffer, nullptr);
    vkFreeMemory(dev.get_device(), stagingbuffer.device_memory, nullptr);
}

void vk::base::set_debug_name(const std::string& name, uint64_t handle, VkObjectType type)
{
    VkDebugUtilsObjectNameInfoEXT info;
	info.pNext = nullptr;
	info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	info.objectHandle = handle;
	info.objectType = type;
	info.pObjectName = name.c_str();
 
    inst.SetDebugUtilsObjectNameEXT(dev.get_device(), &info);
}

void vk::base::unset_debug_render_pass_name(VkCommandBuffer cmd)
{
    inst.SetEndDebugUtilsLabelEXT(cmd);
}

void vk::base::set_debug_render_pass_name(VkCommandBuffer cmd, const std::string& name)
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
    label.pLabelName = name.c_str();
    inst.SetBeginDebugUtilsLabelEXT(cmd, &label);
}

