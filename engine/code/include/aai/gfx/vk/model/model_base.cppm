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
                 
                const VkBuffer get_index_buffer() { return data.indices.buffer; }
                const VkDeviceMemory get_index_device_memory() { return data.indices.device_memory; }
              
                // void update_animation(float delta_ms, const vk::device::handlers& devices);
                //
                // todo_vulkan -- vk::buffer for vertex/index are the same and never chasnges - might be kept here
                // ssbo for skin changes every frame and must probably go to the model::inst member

                struct vk_data {
                    vk::buffer::handlers vertices;
                    vk::buffer::handlers indices;
                };
                const std::vector<model::types::node>& get_nodes() const { return info.nodes; }

                bool is_animated() { return !info.clips.empty(); }
                model::info info;
            private:
                vk_data data;
        };
    }
};
