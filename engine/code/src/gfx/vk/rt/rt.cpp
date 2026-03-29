#include "aai/gfx/vk/backend/vk_defines.h"

import aai.gfx.vk.rt;
import aai.gfx.vk.device;
import aai.gfx.vk.buffer;
import aai.utils;
import aai.utils.mem;

VkImageCreateInfo image_create_info(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent, int layer_count = 1)
{
    VkImageCreateInfo info = { };
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext = nullptr;

    info.imageType = VK_IMAGE_TYPE_2D;

    info.format = format;
    info.extent = extent;
    if (layer_count == 6) {
        info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }
    info.mipLevels = 1;
    info.arrayLayers = layer_count;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = usageFlags;

    return info;
}

VkImageViewCreateInfo image_view_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags, VkImageViewType type = VK_IMAGE_VIEW_TYPE_2D)
{
	VkImageViewCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	info.pNext = nullptr;

	info.viewType = type;
	info.image = image;
	info.format = format;
	info.subresourceRange.baseMipLevel = 0;
	info.subresourceRange.levelCount = 1;
	info.subresourceRange.baseArrayLayer = 0 ;
	info.subresourceRange.layerCount = type == VK_IMAGE_VIEW_TYPE_2D ? 1 : 6;
	info.subresourceRange.aspectMask = aspectFlags;

	return info;
}

void rt::render_target::allocate(const vk::device::handlers& dev)
{
    VkMemoryRequirements memrequirements;
	vkGetImageMemoryRequirements(dev.dev, image, &memrequirements);
    VkMemoryAllocateInfo allocinfo{};
	allocinfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocinfo.allocationSize = memrequirements.size;
	allocinfo.memoryTypeIndex = vk::buffer::find_memory_type(dev.physical_dev, memrequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    utils::VK_ASSERT(vkAllocateMemory(dev.dev, &allocinfo, nullptr, &memory),"failed to allocate image memory!");
	vkBindImageMemory(dev.dev, image, memory, 0);

    utils::mem::get()->track_allocation(utils::mem::resource::TEXTURE, name, memrequirements.size);

	//    VkDebugUtilsObjectNameInfoEXT info;
	// info.pNext = nullptr;
	// info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	// info.objectHandle = reinterpret_cast<uint64_t>(Memory);
	// info.objectType = VK_OBJECT_TYPE_DEVICE_MEMORY;
	// info.pObjectName = "rt buffer";
	// Gfx::Vulkan::GetInstance()->SetDebugName(info);
}

void rt::render_target::clean(const vk::device::handlers& dev)
{
    vkDestroyImage(dev.dev, image, nullptr);
    vkFreeMemory(dev.dev, memory, nullptr);
    vkDestroyImageView(dev.dev, image_view, nullptr);
    vkDestroySampler(dev.dev, sampler, nullptr);

    utils::mem::get()->track_deallocation(utils::mem::resource::TEXTURE, name);
}

void rt::render_target::create(const vk::device::handlers& dev)
{
    VkImageUsageFlags usageflags{};
    VkImageAspectFlags aspectflags{};
    if (is_depth) {
        usageflags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        aspectflags = VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    else {
        usageflags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        aspectflags = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    if (is_storage) {
	    usageflags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }

    VkImageCreateInfo imginfo = image_create_info(format, usageflags, { static_cast<uint32_t>(dimensions.x), static_cast<uint32_t>(dimensions.y), 1 }, is_cube ? 6 : 1);
    utils::VK_ASSERT(vkCreateImage(dev.dev, &imginfo, nullptr, &image), "failed to create image");

    allocate(dev);

    VkImageViewCreateInfo imgviewinfo = image_view_create_info(format, image, aspectflags, is_cube ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D);
	utils::VK_ASSERT(vkCreateImageView(dev.dev, &imgviewinfo, nullptr, &image_view), "failed to create RT image view!");

    if (is_sampler) {
        create_sampler(dev);
    }
    // if (ID == -1) {
    //     Gfx::Vulkan::GetInstance()->SetRTDebugName(Name, Image);
    // }
    // else {
    //     Gfx::Vulkan::GetInstance()->SetRTDebugName(Gfx::GetValue(static_cast<Gfx::RenderTargetsList>(ID)), Image);
    // }
}

void rt::render_target::create_sampler(const vk::device::handlers& dev)
{
    VkSamplerCreateInfo samplerinfo{};
	samplerinfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerinfo.pNext = nullptr;

	samplerinfo.magFilter = VK_FILTER_NEAREST;
	samplerinfo.minFilter = VK_FILTER_NEAREST;
	samplerinfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerinfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerinfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    utils::VK_ASSERT(vkCreateSampler(dev.dev, &samplerinfo, nullptr, &sampler), "failed to create sampler!");
}

void rt::render_target::copy(VkCommandBuffer cmd, VkBuffer buffer,  uint32_t width, uint32_t height, int layer_count)
{
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layer_count;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        static_cast<uint32_t>(width), static_cast<uint32_t>(height),
        1
    };

    vkCmdCopyBufferToImage(cmd, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void rt::render_target::memory_barrier(VkCommandBuffer cmd, VkImageLayout oldlayout, VkImageLayout newlayout, int layer_count)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldlayout;
    barrier.newLayout = newlayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask =  (oldlayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL || newlayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0 ;
    barrier.subresourceRange.layerCount = layer_count;

    VkPipelineStageFlags src;
    VkPipelineStageFlags dst;
    src = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dst = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    src = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    dst= VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

    vkCmdPipelineBarrier(
        cmd,
        src, dst,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
}
