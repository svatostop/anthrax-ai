#include "anthraxAI/gfx/vkrenderer.h"
#include "anthraxAI/gfx/vkdevice.h"
#include "anthraxAI/gfx/bufferhelper.h"

void BufferHelper::CopyBuffer(VkBuffer& srcbuffer, VkBuffer& dstbuffer, VkDeviceSize size)
{
    Gfx::Renderer::GetInstance()->Submit([=](VkCommandBuffer cmd) {
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        vkCmdCopyBuffer(cmd, srcbuffer, dstbuffer, 1, &copyRegion);
    });
}

void BufferHelper::MapMemory(Buffer& buffer, VkDeviceSize size, VkDeviceSize offset, const void* datasrc)
{
    void* datadst;
    vkMapMemory(Gfx::Device::GetInstance()->GetDevice(), buffer.DeviceMemory, offset, size, 0, &datadst);
        memcpy(datadst, datasrc, (size_t)size);
    vkUnmapMemory(Gfx::Device::GetInstance()->GetDevice(), buffer.DeviceMemory);
}

void BufferHelper::AllocateBuffer(Buffer& bufhandler, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_ASSERT(vkCreateBuffer(Gfx::Device::GetInstance()->GetDevice(), &bufferInfo, nullptr, &bufhandler.Buffer), "failed to create buffer!");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(Gfx::Device::GetInstance()->GetDevice(), bufhandler.Buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(Gfx::Device::GetInstance()->GetPhysicalDevice(), memRequirements.memoryTypeBits, properties);

    VK_ASSERT(vkAllocateMemory(Gfx::Device::GetInstance()->GetDevice(), &allocInfo, nullptr, &bufhandler.DeviceMemory), "failed to allocate buffer memory!");
    vkBindBufferMemory(Gfx::Device::GetInstance()->GetDevice(), bufhandler.Buffer, bufhandler.DeviceMemory, 0);
}

void BufferHelper::CreateBuffer(Buffer& bufferhandler, VkBufferUsageFlags flags[2], VkDeviceSize buffersize, const void *datasrc)
{
    Buffer stagingbuffer;
    AllocateBuffer(stagingbuffer, buffersize, flags[0], VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    MapMemory(stagingbuffer, buffersize, 0, datasrc);

    AllocateBuffer(bufferhandler, buffersize, flags[1], VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    CopyBuffer(stagingbuffer.Buffer, bufferhandler.Buffer, buffersize);

    vkDestroyBuffer(Gfx::Device::GetInstance()->GetDevice(), stagingbuffer.Buffer, nullptr);
    vkFreeMemory(Gfx::Device::GetInstance()->GetDevice(), stagingbuffer.DeviceMemory, nullptr);
}

void BufferHelper::CreateBuffer(Buffer& bufferhandler, VkDeviceSize buffersize, VkBufferUsageFlagBits usage, bool device_only )
{
    if (device_only) {
        // usage = static_cast<VkBufferUsageFlagBits>(usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
            AllocateBuffer(bufferhandler, buffersize, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }
    else {
        if (usage == VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) {
            AllocateBuffer(bufferhandler, buffersize, usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        }
        else {
            AllocateBuffer(bufferhandler, buffersize, usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        }
    }
}
void BufferHelper::CreateTextureBuffer(Buffer& bufferhandler, VkDeviceSize buffersize, void *pixels)
{
}
uint32_t BufferHelper::FindMemoryType(VkPhysicalDevice physicaldevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicaldevice, &memProperties);
    uint32_t i = 0;
    for (; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    ASSERT(i != 0, "failed to find suitable memory type!");
    return 0;
}
