module;
#include "aai/gfx/vk/backend/vk_defines.h"

export module aai.gfx.vk.gpu_memory;
export import aai.gfx.vk.gpu_data;
export import aai.gfx.vk.buffer;
export import aai.gfx.vk.device;

export {
    namespace vk {
        class gpu_memory {
            public:
                void init(vk::device::handlers dev);

                VkDescriptorSetLayout get_bindless_layout() { return bindless_texture_layout; }
            private:
                void init_descriptor_set(vk::device::handlers dev);
                void init_buffers(vk::device::handlers dev);

                VkDescriptorPool texture_pool;
	            VkDescriptorSetLayout bindless_texture_layout = VK_NULL_HANDLE;
                VkDescriptorSet bindless_texture_descriptor;

                vk::buffer::handlers camera;
        };
    }
};
