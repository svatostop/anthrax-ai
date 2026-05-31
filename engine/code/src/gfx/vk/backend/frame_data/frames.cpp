#include "aai/gfx/vk/backend/vk_defines.h"
#include <cstdint>

import aai.gfx.vk.frames;
import aai.gfx.vk.rt.cmd;
import aai.utils;

void vk::frames::init(VkDevice dev, const uint32_t graphics_index, const uint32_t swapchain_size)
{
    cmd.init(dev, graphics_index); 
    sync.init(dev, swapchain_size);
}

bool vk::frames::sync_frames(VkDevice dev, VkSwapchainKHR swapchain)
{
    if (!sync.sync_frames(dev, swapchain, frame_index))
        return false;
    cmd.sync_frames(frame_index);
    cmd.begin(frame_index);
    return true;
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

bool vk::frames::submit_and_present(VkQueue queue, VkSwapchainKHR swapchain)
{
    const uint64_t graphics_finished = sync.get_timeline_value();
    const uint64_t all_finished = sync.get_timeline_value() + 1;

    VkCommandBuffer cmd_submit = cmd.get(frame_index);
    VkSemaphore render_sema = sync.get_render_sema();
    VkSemaphore wait_sema = sync.get_wait_sema(frame_index);
    VkPipelineStageFlags graphics_wait_masks[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore graphics_wait_sema[] = { sync.get_timeline(), wait_sema };
    VkSemaphore graphics_signal_sema[] = { sync.get_timeline(), render_sema };
    VkSubmitInfo submit = {};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd_submit;
    submit.waitSemaphoreCount = 2;
    submit.pWaitSemaphores = graphics_wait_sema;
    submit.pWaitDstStageMask = graphics_wait_masks;
    submit.signalSemaphoreCount = 2;
    submit.pSignalSemaphores = graphics_signal_sema;
    uint64_t wait_values[2] = { graphics_finished , graphics_finished };
    uint64_t signal_values[2] = { all_finished , all_finished };		
    VkTimelineSemaphoreSubmitInfoKHR timeline_submit{ VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO_KHR };
    timeline_submit.waitSemaphoreValueCount = 2;
    timeline_submit.pWaitSemaphoreValues = &wait_values[0];
    timeline_submit.signalSemaphoreValueCount = 2;
    timeline_submit.pSignalSemaphoreValues = &signal_values[0];

    submit.pNext = &timeline_submit;
    utils::VK_ASSERT(vkQueueSubmit(queue, 1, &submit, VK_NULL_HANDLE), "failed to submit queue!");
    sync.set_timeline_value(all_finished);
    
    uint32_t swap_ind = sync.get_swapchain_index();
    VkPresentInfoKHR prinfo = present_info(
		&swapchain,
		&render_sema,
		sync.get_swapchain_index_ptr()
	);
	VkResult presentresult = vkQueuePresentKHR(queue, &prinfo);

    bool res = true;
	if (presentresult == VK_ERROR_OUT_OF_DATE_KHR) {
	     res = false; 
	}

   inc_frame_ind();
   return res;
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

void vk::frames::submit(VkDevice dev, VkQueue queue, std::function<void(VkCommandBuffer cmd)>&& f)
{
	VkCommandBuffer cmd = get_upload_cmd();
	VkCommandBufferBeginInfo cmdBeginInfo = cmd_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    utils::VK_ASSERT(vkBeginCommandBuffer(cmd, &cmdBeginInfo), "failed to begin command buffer!");
	f(cmd);
    utils::VK_ASSERT(vkEndCommandBuffer(cmd), "failed to end command buffer!");

	VkSubmitInfo submitinfo = submit_info(&cmd);
    utils::VK_ASSERT(vkQueueSubmit(queue, 1, &submitinfo, *(get_upload_fence())), "failed to submit upload queue!");

	vkWaitForFences(dev, 1, get_upload_fence(), true, 9999999999);
	vkResetFences(dev, 1, get_upload_fence());
	vkResetCommandPool(dev, get_upload_cmd_pool(), 0);
}

