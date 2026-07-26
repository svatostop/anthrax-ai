module;
#include "aai/gfx/vk/model/model_types.h"
#include <vulkan/vulkan_core.h>

export module aai.gfx.vk.model;
export import aai.gfx.vk.buffer;
export import aai.gfx.vk.device;
export import std;
export {
    namespace model {
        class base {
            public:
                void load(const std::string& path, vk::device::handlers devices);

                const VkBuffer get_vertex_buffer() { return data.vertices.buffer; }
                const VkDeviceMemory get_vertex_device_memory() { return data.vertices.device_memory; }
                
                const VkDeviceAddress get_skin_gpu_address(const uint32_t id) { return data.skin[id].gpu_address; }
                
                const VkBuffer get_index_buffer() { return data.indices.buffer; }
                const VkDeviceMemory get_index_device_memory() { return data.indices.device_memory; }
               
                void update_animation(const vk::device::handlers& devices);
                struct vk_data {
                    vk::buffer::handlers vertices;
                    vk::buffer::handlers indices;
                    std::vector<vk::buffer::handlers> skin;
                };
                const std::vector<model::types::node*>& get_nodes() const { return nodes; }
            private:
                std::vector<model::types::node*> nodes;
                std::vector<model::types::skin> skins;
                // todo - separate from model - some models can have the same animation
                std::vector<model::animation::base> animations;	
                uint32_t active_anim = 0;
                vk_data data;
        };
    }
};
