#include "aai/gfx/vk/backend/vk_defines.h"
import aai.gfx.vk.frames.sync;
import aai.utils;
import aai.utils.mem;
import std;

VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags)
{
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.pNext = nullptr;
    fenceCreateInfo.flags = flags;
    return fenceCreateInfo;
}

VkSemaphoreCreateInfo semaphore_create_info(VkSemaphoreCreateFlags flags)
{
    VkSemaphoreCreateInfo semCreateInfo = {};
    semCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semCreateInfo.pNext = nullptr;
    semCreateInfo.flags = flags;
    return semCreateInfo;
}
void vk::synchronization::init(VkDevice dev)
{
    swapchain_index = 0;
   	VkFenceCreateInfo fencecreateinfo = fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
	VkSemaphoreCreateInfo semcreateinfo = semaphore_create_info(0);

	VkFenceCreateInfo uploadfencecreateinfo = fence_create_info(0);
    utils::VK_ASSERT(vkCreateFence(dev, &uploadfencecreateinfo, nullptr, &upload_fence), "failed to create upload fence !");
	utils::mem::get()->push(utils::mem::event::DELETE, utils::mem::type::VK, [=,this]() {
        vkDestroyFence(dev, upload_fence, nullptr);
	});
	for (int i = 0; i < MAX_FRAMES + 1; i++) {
	    utils::VK_ASSERT(vkCreateFence(dev, &fencecreateinfo, nullptr, &render_fence[i]), "failed to create fence !");

	    utils::VK_ASSERT(vkCreateSemaphore(dev, &semcreateinfo, nullptr, &present_sema[i]), "failed to create present semaphore!");
	    utils::VK_ASSERT(vkCreateSemaphore(dev, &semcreateinfo, nullptr, &render_sema[i]), "failed to create render semaphore!");

		utils::mem::get()->push(utils::mem::event::DELETE, utils::mem::type::VK, [=,this]() {
			vkDestroyFence(dev, render_fence[i], nullptr);
			vkDestroySemaphore(dev, present_sema[i], nullptr);
			vkDestroySemaphore(dev, render_sema[i], nullptr);
		});
	}
}
