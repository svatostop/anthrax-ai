module;
#include "aai/gfx/vk/backend/vk_defines.h"
#include <cstring>

export module aai.gfx.vk.buffer;
import aai.gfx.vk.device;
import aai.utils.mem;
import std;

export {
namespace vk {
    namespace buffer 
    {
        struct handlers {
            VkBuffer buffer;
            VkDeviceMemory device_memory;
            void* uniform_mapped_memory;
            std::string tag;
        };
        
        void copy(VkBuffer& srcbuffer, VkBuffer& dstbuffer,VkDeviceSize size)
        {
            // Gfx::Renderer::GetInstance()->Submit([=](VkCommandBuffer cmd) {
            // VkBufferCopy copyRegion{};
            // copyRegion.srcOffset = 0;
            // copyRegion.dstOffset = 0;
            // copyRegion.size = size;
            // vkCmdCopyBuffer(cmd, srcbuffer, dstbuffer, 1, &copyRegion);
            // });
        }
        uint32_t find_memory_type(VkPhysicalDevice physicaldevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
        {
            VkPhysicalDeviceMemoryProperties memProperties;
            vkGetPhysicalDeviceMemoryProperties(physicaldevice, &memProperties);
            uint32_t i = 0;
            for (; i < memProperties.memoryTypeCount; i++) {
                if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }
            utils::ASSERT(i != 0, "failed to find suitable memory type!");
            return 0;
        }    

        void map_memory(vk::buffer::handlers& buffer, VkDevice dev, VkDeviceSize size, VkDeviceSize offset, const void* datasrc)
        {
            void* datadst;
            vkMapMemory(dev, buffer.device_memory, offset, size, 0, &datadst);
                memcpy(datadst, datasrc, (size_t)size);
            vkUnmapMemory(dev, buffer.device_memory);
        }

        void allocate(vk::buffer::handlers& bufhandler, vk::device::handlers devices, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
        {
            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = size;
            bufferInfo.usage = usage;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            utils::VK_ASSERT(vkCreateBuffer(devices.dev, &bufferInfo, nullptr, &bufhandler.buffer), "failed to create buffer!");

            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(devices.dev, bufhandler.buffer, &memRequirements);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = find_memory_type(devices.physical_dev, memRequirements.memoryTypeBits, properties);

            utils::VK_ASSERT(vkAllocateMemory(devices.dev, &allocInfo, nullptr, &bufhandler.device_memory), "failed to allocate buffer memory!");
            vkBindBufferMemory(devices.dev, bufhandler.buffer, bufhandler.device_memory, 0);
        }

        void create(vk::buffer::handlers& bufferhandler, vk::device::handlers devices, VkBufferUsageFlags flags[2], VkDeviceSize buffersize, const void *datasrc)
        {
            handlers stagingbuffer;
            allocate(stagingbuffer, devices, buffersize, flags[0], VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            map_memory(stagingbuffer, devices.dev, buffersize, 0, datasrc);

            allocate(bufferhandler, devices, buffersize, flags[1], VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        // todo: doesn't submit
            copy(stagingbuffer.buffer, bufferhandler.buffer, buffersize);

            vkDestroyBuffer(devices.dev, stagingbuffer.buffer, nullptr);
            vkFreeMemory(devices.dev, stagingbuffer.device_memory, nullptr);
        }

        void allocate_with_mem_type(vk::buffer::handlers& bufferhandler, vk::device::handlers devices, VkDeviceSize buffersize, VkBufferUsageFlagBits usage, bool device_only )
        {
            if (device_only) {
                // usage = static_cast<VkBufferUsageFlagBits>(usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
                    allocate(bufferhandler, devices,  buffersize, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            }
            else {
                if (usage == VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) {
                    allocate(bufferhandler, devices, buffersize, usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                }
                else {
                    allocate(bufferhandler,devices,  buffersize, usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                }
            }
        }
    }
}
};
