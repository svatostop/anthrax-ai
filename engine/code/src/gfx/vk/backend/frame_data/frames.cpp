#include "aai/gfx/vk/backend/vk_defines.h"
#include <cstdint>
import aai.gfx.vk.frames;

void vk::frames::init(VkDevice dev, const uint32_t graphics_index)
{
    cmd.init(dev, graphics_index); 
    sync.init(dev);
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

