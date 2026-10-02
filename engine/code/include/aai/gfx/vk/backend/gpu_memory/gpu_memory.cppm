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

                const VkDeviceAddress get_buffer_address(const gpu_data_type& t, const uint32_t frame) const;

                void submit_camera(std::shared_ptr<const keeper::camera> cam, const std::uint32_t frame);
                void submit_instance(const std::deque<rq::data>& rq, const std::uint32_t frame);
                void update(vk::device::handlers dev, const std::uint32_t frame);
            private:
                void init_descriptor_set(vk::device::handlers dev);
                void init_buffers(vk::device::handlers dev);

                VkDescriptorPool texture_pool;
	            VkDescriptorSetLayout bindless_texture_layout = VK_NULL_HANDLE;
                VkDescriptorSet bindless_texture_descriptor;

                gpu_data<camera_data> camera[MAX_FRAMES];
                gpu_data<instance_data> instance[MAX_FRAMES];

                std::map<std::string, uint32_t> texture_bindings;
                uint32_t texture_handle = 0;
        };
    }
};
