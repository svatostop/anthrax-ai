#include "aai/gfx/vk/backend/vk_defines.h"
#include <stdio.h>
#include <vulkan/vulkan_core.h>
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

void vk::synchronization::wait_timeline(VkDevice dev)
{
    VkSemaphoreWaitInfo sema_wait{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO };
    sema_wait.semaphoreCount = 1;
    sema_wait.pSemaphores = &timeline.handle;
    sema_wait.pValues = &timeline.value;
    vkWaitSemaphores(dev, &sema_wait, UINT64_MAX);
}

bool vk::synchronization::sync_frames(VkDevice dev, VkSwapchainKHR swapchain, uint32_t frame_index)
{
   	VkResult e = vkAcquireNextImageKHR(dev, swapchain, 1000000000, present_sema[frame_index], VK_NULL_HANDLE, &swapchain_index);
	if (e == VK_ERROR_OUT_OF_DATE_KHR) {
        swapchain_index = -1;
        return false;     
    }
    wait_timeline(dev);
    return true;
}

void vk::synchronization::init(VkDevice dev, const int sw_size)
{		
    swapchain_index = 0;
    swapchain_size = sw_size;
   	VkFenceCreateInfo fencecreateinfo = fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
	VkSemaphoreCreateInfo semcreateinfo = semaphore_create_info(0);

	VkFenceCreateInfo uploadfencecreateinfo = fence_create_info(0);
    utils::VK_ASSERT(vkCreateFence(dev, &uploadfencecreateinfo, nullptr, &upload_fence), "failed to create upload fence !");
	utils::mem::get()->push(utils::mem::event::DELETE, utils::mem::type::VK, [=,this]() {
        vkDestroyFence(dev, upload_fence, nullptr);
	});

    present_sema.resize(MAX_FRAMES);
    render_sema.resize(swapchain_size);
	for (int i = 0; i < MAX_FRAMES; i++) {
	    utils::VK_ASSERT(vkCreateSemaphore(dev, &semcreateinfo, nullptr, &present_sema[i]), "failed to create present semaphore!");
        if (i < swapchain_size) {
	        utils::VK_ASSERT(vkCreateSemaphore(dev, &semcreateinfo, nullptr, &render_sema[i]), "failed to create render semaphore!");
            utils::mem::get()->push(utils::mem::event::DELETE, utils::mem::type::VK, [=,this]() {
                vkDestroySemaphore(dev, render_sema[i], nullptr);
            });
        }
		utils::mem::get()->push(utils::mem::event::DELETE, utils::mem::type::VK, [=,this]() {
			vkDestroySemaphore(dev, present_sema[i], nullptr);
		});
	}
	for (int i = MAX_FRAMES; i < swapchain_size; i++) {
        utils::VK_ASSERT(vkCreateSemaphore(dev, &semcreateinfo, nullptr, &render_sema[i]), "failed to create present semaphore!");
		utils::mem::get()->push(utils::mem::event::DELETE, utils::mem::type::VK, [=,this]() {
			vkDestroySemaphore(dev, render_sema[i], nullptr);
        });
    }
    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkSemaphoreTypeCreateInfoKHR sema_type_info{};
    sema_type_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO_KHR;
    sema_type_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
    sema_type_info.initialValue = timeline.value;
    semaphore_info.pNext = &sema_type_info;
    utils::VK_ASSERT(vkCreateSemaphore(dev, &semaphore_info, nullptr, &timeline.handle), "failed to create timeline sema!");
    utils::mem::get()->push(utils::mem::event::DELETE, utils::mem::type::VK, [=,this]() {
            vkDestroySemaphore(dev, timeline.handle, nullptr);
            });
}
