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
        std::function<void(VkDevice dev, VkQueue queue, std::function<void(VkCommandBuffer cmd)>&& f)> submit_callback;
        VkQueue callback_queue;
        
        struct handlers {
            VkBuffer buffer;
            VkDeviceMemory device_memory;
            std::string tag;

            VkDeviceAddress gpu_address = 0; 
        };
        
        void copy(VkDevice dev, VkBuffer& srcbuffer, VkBuffer& dstbuffer,VkDeviceSize size)
        {
            utils::ASSERT(!submit_callback, "submit callback was null");

            submit_callback(dev, callback_queue, [=](VkCommandBuffer cmd) {
                VkBufferCopy copyRegion{};
                copyRegion.srcOffset = 0;
                copyRegion.dstOffset = 0;
                copyRegion.size = size;
                vkCmdCopyBuffer(cmd, srcbuffer, dstbuffer, 1, &copyRegion);
            });
        }
        void get_gpu_address(handlers& handle, VkDevice dev)
        {
            VkBufferDeviceAddressInfoKHR address_info{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR};
	        address_info.buffer = handle.buffer;
	        handle.gpu_address  = vkGetBufferDeviceAddress(dev, &address_info);
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
            
            if (usage == VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
                VkMemoryAllocateFlagsInfoKHR flags_info{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR};
	            flags_info.flags  = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
	            allocInfo.pNext = &flags_info;
            }
            utils::VK_ASSERT(vkAllocateMemory(devices.dev, &allocInfo, nullptr, &bufhandler.device_memory), "failed to allocate buffer memory!");
            vkBindBufferMemory(devices.dev, bufhandler.buffer, bufhandler.device_memory, 0);
        }

        void create(vk::buffer::handlers& bufferhandler, vk::device::handlers devices, VkBufferUsageFlags flags[2], VkDeviceSize buffersize, const void *datasrc)
        {
            handlers stagingbuffer;
            allocate(stagingbuffer, devices, buffersize, flags[0], VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            map_memory(stagingbuffer, devices.dev, buffersize, 0, datasrc);

            allocate(bufferhandler, devices, buffersize, flags[1], VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            
            copy(devices.dev, stagingbuffer.buffer, bufferhandler.buffer, buffersize);

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
        void set_submit_callback(std::function<void(VkDevice dev, VkQueue queue, std::function<void(VkCommandBuffer cmd)>&& f)>&& callback, VkQueue queue) { submit_callback = callback; callback_queue = queue; }
    }
}
};
