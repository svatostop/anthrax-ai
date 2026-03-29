module;
#include "aai/gfx/vk/backend/vk_defines.h"

export module aai.gfx.vk.descriptors;

export {
    namespace vk {
        class descriptors {
            public:
                void init(VkDevice dev);
            private:
                VkDescriptorPool buffer_pool[MAX_FRAMES];
	            VkDescriptorSetLayout bindless_buffer_layout = VK_NULL_HANDLE;
                VkDescriptorSet bindless_buffer_descriptor[MAX_FRAMES];

                VkDescriptorPool texture_pool;
	            VkDescriptorSetLayout bindless_texture_layout = VK_NULL_HANDLE;
                VkDescriptorSet bindless_texture_descriptor;
        };
    }
};
