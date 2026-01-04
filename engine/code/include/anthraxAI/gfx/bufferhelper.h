#pragma once
#include "anthraxAI/gfx/vkdefines.h"

namespace BufferHelper
{
    struct Buffer {
        VkBuffer Buffer;
        VkDeviceMemory DeviceMemory;
        void* UniformMappedMemory;
        std::string tag;
    };

    void    MapMemory(Buffer& buffer, VkDeviceSize size, VkDeviceSize offset, const void* datasrc);

    void 	CopyBuffer(VkBuffer& srcbuffer, VkBuffer& dstbuffer, VkDeviceSize size);
	void 	AllocateBuffer(Buffer& bufhandler, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
	void 	CreateBuffer(Buffer& bufferhandler, VkBufferUsageFlags flags[2], VkDeviceSize buffersize, const void *datasrc);

	void 	CreateBuffer(Buffer& bufferhandler, VkDeviceSize buffersize, VkBufferUsageFlagBits usage, bool device_only = false);
	void  	CreateTextureBuffer(Buffer& bufferhandler, VkDeviceSize buffersize, void *pixels);
	uint32_t FindMemoryType(VkPhysicalDevice physicaldevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
}
