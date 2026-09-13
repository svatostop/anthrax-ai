module;
#include "aai/gfx/vk/backend/vk_defines.h"

export module aai.gfx.vk.gpu_memory;
export import aai.keeper.camera;
export import aai.gfx.vk.gpu_data;
export import aai.gfx.vk.buffer;
export import aai.gfx.vk.device;
export import aai.gfx.vk.rq;
export import std;
export import glm;
export {
    namespace vk {
        class gpu_memory {
            public:
                void init(vk::device::handlers dev);
                
                void update_texture(VkDevice dev, const std::string& name, VkImageView view, VkSampler sampler);

                VkDescriptorSetLayout get_bindless_layout() { return bindless_texture_layout; }
                VkDescriptorSet get_bindless_set() { return bindless_texture_descriptor; }

                VkDeviceAddress get_buffer_address(const gpu_data_type& t);

                void submit_camera(std::shared_ptr<const keeper::camera> cam);
                void submit_instance(const std::deque<rq::data>& rq);
                void update(vk::device::handlers dev);
            private:
                void init_descriptor_set(vk::device::handlers dev);
                void init_buffers(vk::device::handlers dev);

                VkDescriptorPool texture_pool;
	            VkDescriptorSetLayout bindless_texture_layout = VK_NULL_HANDLE;
                VkDescriptorSet bindless_texture_descriptor;

                gpu_data<camera_data> camera;
                gpu_data<instance_data> instance;

                std::map<std::string, uint32_t> texture_bindings;
                uint32_t texture_handle = 0;
        };
    }
};
