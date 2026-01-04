#include "anthraxAI/gfx/vkrendertarget.h"
#include "anthraxAI/gfx/renderhelpers.h"
#include "anthraxAI/gfx/vkdevice.h"
#include "anthraxAI/gfx/vkrenderer.h"
#include <cstdint>
#include <vulkan/vulkan_core.h>

Gfx::RenderTarget::RenderTarget(const RenderTarget& rt, uint32_t id) :
    Format(rt.Format), Dimensions(rt.Dimensions),
    IsDepth(rt.IsDepth), IsStorage(rt.IsStorage), ID(id)
{
    Name = Gfx::GetValue(static_cast<RenderTargetsList>(id));
}

VkImageCreateInfo ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent, int layer_count = 1)
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

VkImageViewCreateInfo ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags, VkImageViewType type = VK_IMAGE_VIEW_TYPE_2D)
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

void Gfx::RenderTarget::AllocateRTMemory()
{
    VkMemoryRequirements memrequirements;
	vkGetImageMemoryRequirements(Gfx::Device::GetInstance()->GetDevice(), Image, &memrequirements);
    VkMemoryAllocateInfo allocinfo{};
	allocinfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocinfo.allocationSize = memrequirements.size;
	allocinfo.memoryTypeIndex = BufferHelper::FindMemoryType(Gfx::Device::GetInstance()->GetPhysicalDevice(), memrequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VK_ASSERT(vkAllocateMemory(Gfx::Device::GetInstance()->GetDevice(), &allocinfo, nullptr, &Memory),"failed to allocate image memory!");
	vkBindImageMemory(Gfx::Device::GetInstance()->GetDevice(), Image, Memory, 0);

    VkDebugUtilsObjectNameInfoEXT info;
	info.pNext = nullptr;
	info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	info.objectHandle = reinterpret_cast<uint64_t>(Memory);
	info.objectType = VK_OBJECT_TYPE_DEVICE_MEMORY;
	info.pObjectName = "rt buffer";
	Gfx::Vulkan::GetInstance()->SetDebugName(info);
}

void Gfx::RenderTarget::CreateRenderTarget()
{
    VkImageUsageFlags usageflags{};
    VkImageAspectFlags aspectflags{};
    if (IsDepth) {
        usageflags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        aspectflags = VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    else {
        usageflags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        aspectflags = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    if (IsStorage) {
	    usageflags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }

    VkImageCreateInfo imginfo = ImageCreateInfo(Format, usageflags, { static_cast<uint32_t>(Dimensions.x), static_cast<uint32_t>(Dimensions.y), 1 }, IsCube ? 6 : 1);
    VK_ASSERT(vkCreateImage(Gfx::Device::GetInstance()->GetDevice(), &imginfo, nullptr, &Image), "failed to create image");

    AllocateRTMemory();

    VkImageViewCreateInfo imgviewinfo = ImageViewCreateInfo(Format, Image, aspectflags, IsCube ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D);
	VK_ASSERT(vkCreateImageView(Gfx::Device::GetInstance()->GetDevice(), &imgviewinfo, nullptr, &ImageView), "failed to create RT image view!");
    if (ID == -1) {
        Gfx::Vulkan::GetInstance()->SetRTDebugName(Name, Image);
    }
    else {
        Gfx::Vulkan::GetInstance()->SetRTDebugName(Gfx::GetValue(static_cast<Gfx::RenderTargetsList>(ID)), Image);
    }
}

void Gfx::RenderTarget::MemoryBarrier(VkCommandBuffer cmd, VkImageLayout oldlayout, VkImageLayout newlayout, int layer_count )
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldlayout;
    barrier.newLayout = newlayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = Image;
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

void Gfx::RenderTarget::Copy(VkCommandBuffer cmd, VkBuffer buffer, uint32_t width, uint32_t height, VkDeviceSize layer_size, int layer_count)
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

    vkCmdCopyBufferToImage(cmd, buffer, Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

}
void Gfx::RenderTarget::Copy(VkCommandBuffer cmd, VkBuffer buffer, uint32_t width, uint32_t height)
{
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        static_cast<uint32_t>(width), static_cast<uint32_t>(height),
        1
    };

    vkCmdCopyBufferToImage(cmd, buffer, Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void Gfx::RenderTarget::Clean()
{
    vkDestroyImage(Gfx::Device::GetInstance()->GetDevice(), Image, nullptr);
    vkFreeMemory(Gfx::Device::GetInstance()->GetDevice(), Memory, nullptr);
    vkDestroyImageView(Gfx::Device::GetInstance()->GetDevice(), ImageView, nullptr);
    vkDestroySampler(Gfx::Device::GetInstance()->GetDevice(), Sampler, nullptr);
}
