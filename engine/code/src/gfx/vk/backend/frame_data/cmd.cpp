module;
#include "aai/gfx/vk/backend/vk_defines.h"
#include <cstdint>

module aai.gfx.vk.frames.cmd;
import aai.utils;
import aai.utils.mem;
import std;

VkCommandPoolCreateInfo cmd_pool_create_info(uint32_t graphicsfamily, VkCommandPoolCreateFlags flags) {
	VkCommandPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.pNext = nullptr;

	info.queueFamilyIndex = graphicsfamily;
	info.flags = flags;
	return info;
}
VkCommandBufferAllocateInfo cmd_create_info(VkCommandPool pool, uint32_t count, VkCommandBufferLevel level) {
	VkCommandBufferAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.pNext = nullptr;

	info.commandPool = pool;
	info.commandBufferCount = count;
	info.level = level;
	return info;
}

VkCommandBufferBeginInfo begin_cmd_info(VkCommandBufferUsageFlags flags)
{
    VkCommandBufferBeginInfo cmdbegininfo = {};
	cmdbegininfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdbegininfo.pNext = nullptr;

	cmdbegininfo.pInheritanceInfo = nullptr;
	cmdbegininfo.flags = flags;
    return cmdbegininfo;
}

void vk::command_buffer::end(uint32_t index)
{
    utils::VK_ASSERT(vkEndCommandBuffer(main_cmd[index]), "failed to end command buffer");
}

void vk::command_buffer::begin(uint32_t index)
{
    VkCommandBufferBeginInfo info = begin_cmd_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    utils::VK_ASSERT(vkBeginCommandBuffer(main_cmd[index], &info), "failed to begin a command buffer!");
}

void vk::command_buffer::sync_frames(uint32_t index) const
{
    utils::VK_ASSERT(vkResetCommandBuffer(main_cmd[index], 0), "vkResetCommandBuffer failed!");
}

void vk::command_buffer::init(VkDevice dev, const uint32_t graphics_index)
{
    VkCommandPoolCreateInfo poolinfo = cmd_pool_create_info(graphics_index, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    for (int i = 0; i < MAX_FRAMES; i++) {
        utils::VK_ASSERT(vkCreateCommandPool(dev, &poolinfo, nullptr, &main_cmd_pool[i]), "failed to create command pool!");

		VkCommandBufferAllocateInfo cmdinfo = cmd_create_info(main_cmd_pool[i], 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        utils::VK_ASSERT(vkAllocateCommandBuffers(dev, &cmdinfo, &main_cmd[i]), "failed to allocate command buffers!");
        utils::mem::get()->push(utils::mem::event::DELETE, utils::mem::type::VK, [=,this]() { vkDestroyCommandPool(dev, main_cmd_pool[i], nullptr);});
	}

    utils::VK_ASSERT(vkCreateCommandPool(dev, &poolinfo, nullptr, &upload_cmd_pool), "failed to create upload command pool!");
    utils::mem::get()->push(utils::mem::event::DELETE, utils::mem::type::VK, [=,this]() { vkDestroyCommandPool(dev, upload_cmd_pool, nullptr); });

	VkCommandBufferAllocateInfo cmdallocinfo = cmd_create_info(upload_cmd_pool, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    utils::VK_ASSERT(vkAllocateCommandBuffers(dev, &cmdallocinfo, &upload_cmd), "failed to allocate upload command buffers!");
}
