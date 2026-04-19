#include "aai/gfx/vk/backend/vk_defines.h"

import aai.gfx.vk.frames;
import aai.gfx.vk.rt.helper;
import aai.utils;

void vk::frames::init(VkDevice dev, const uint32_t graphics_index)
{
    cmd.init(dev, graphics_index); 
    sync.init(dev);
}
void vk::frames::sync_frames(VkDevice dev, VkSwapchainKHR swapchain)
{
    sync.sync_frames(dev, swapchain);
    cmd.sync_frames(frame_index);
    cmd.begin(frame_index);
}

void vk::frames::prepare_for_present(VkImage src_image, const glm::vec2& src_size, VkImage sw_image, const glm::vec2& sw_size)
{
    rt::helper::copy_image(cmd.get(frame_index),
        src_image, src_size, 
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        sw_image, sw_size,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    rt::helper::memory_barrier(cmd.get(frame_index), src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    rt::helper::memory_barrier(cmd.get(frame_index), sw_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    cmd.end(frame_index);
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

