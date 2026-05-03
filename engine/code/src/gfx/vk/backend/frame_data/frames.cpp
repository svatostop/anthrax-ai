#include "aai/gfx/vk/backend/vk_defines.h"
#include <cstdint>
#include <vulkan/vulkan_core.h>

import aai.gfx.vk.frames;
import aai.gfx.vk.rt.cmd;
import aai.utils;

void vk::frames::init(VkDevice dev, const uint32_t graphics_index)
{
    cmd.init(dev, graphics_index); 
    sync.init(dev);
}
void vk::frames::sync_frames(VkDevice dev, VkSwapchainKHR swapchain)
{
    sync.sync_frames(dev, swapchain, frame_index);
    cmd.sync_frames(frame_index);
    cmd.begin(frame_index);
}

void vk::frames::prepare_for_present(VkImage src_image, const glm::vec2& src_size, VkImage sw_image, const glm::vec2& sw_size)
{
    rt::cmd::copy_image(cmd.get(frame_index),
        src_image, src_size, 
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        sw_image, sw_size,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    rt::cmd::memory_barrier(cmd.get(frame_index), src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    rt::cmd::memory_barrier(cmd.get(frame_index), sw_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    cmd.end(frame_index);
}

VkPresentInfoKHR present_info(VkSwapchainKHR* swapchain, VkSemaphore* rendersem, uint32_t* swapchind)
{
    VkPresentInfoKHR presentinfo = {};
	presentinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentinfo.pNext = nullptr;
	presentinfo.pSwapchains = swapchain;
	presentinfo.swapchainCount = 1;
	presentinfo.pWaitSemaphores = rendersem;
	presentinfo.waitSemaphoreCount = 1;
	presentinfo.pImageIndices = swapchind;

    return presentinfo;
}

void vk::frames::submit_and_present(VkQueue queue, VkSwapchainKHR swapchain)
{
    VkSubmitInfo submit = {};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.pNext = nullptr;
	VkPipelineStageFlags waitstage2 =VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;// VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;//VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submit.pWaitDstStageMask = &waitstage2;
	submit.waitSemaphoreCount = 1;
    VkSemaphore wait_sema = sync.get_wait_sema(frame_index);
	submit.pWaitSemaphores = &wait_sema;
	submit.signalSemaphoreCount = 1;
    VkSemaphore render_sema = sync.get_render_sema(frame_index);
	submit.pSignalSemaphores = &render_sema;
	submit.commandBufferCount = 1;
    VkCommandBuffer cmd_submit = cmd.get(frame_index);
	submit.pCommandBuffers = &cmd_submit;
    utils::VK_ASSERT(vkQueueSubmit(queue, 1, &submit, sync.get_render_fence(frame_index)), "failed to submit queue!");
    
    uint32_t swap_ind = sync.get_swapchain_index();
    VkPresentInfoKHR prinfo = present_info(
		&swapchain,
		&render_sema,
		sync.get_swapchain_index_ptr()
	);
	VkResult presentresult = vkQueuePresentKHR(queue, &prinfo);

	// if (presentresult == VK_ERROR_OUT_OF_DATE_KHR) {
	//       OnResize = true;
	// }

   inc_frame_ind();
}

VkCommandBufferBeginInfo vk::frames::cmd_begin_info(VkCommandBufferUsageFlags flags)
{
    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext = nullptr;

    info.pInheritanceInfo = nullptr;
    info.flags = flags;
    return info;
}

VkSubmitInfo vk::frames::submit_info(VkCommandBuffer* cmd)
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

