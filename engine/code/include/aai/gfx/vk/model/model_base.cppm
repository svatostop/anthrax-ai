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
                void load(const std::string& p, vk::device::handlers devices);
                void load_simple(const std::string& p, const std::vector<types::vertex>& vertex_buffer, const std::vector<uint16_t>& index_buffer, vk::device::handlers devices);

                const VkBuffer get_vertex_buffer() { return data.vertices.buffer; }
                const VkDeviceMemory get_vertex_device_memory() { return data.vertices.device_memory; }
                 
                const VkBuffer get_index_buffer() { return data.indices.buffer; }
                const VkDeviceMemory get_index_device_memory() { return data.indices.device_memory; }
              
                struct vk_data {
                    vk::buffer::handlers vertices;
                    vk::buffer::handlers indices;
                };
                const std::vector<model::types::node>& get_nodes() const { return info.nodes; }

                bool is_animated() { return !info.clips.empty(); }
                const std::string& get_path() { return path; }
                model::info info;
            private:
                vk_data data;
                std::string path;

                std::vector<uint16_t> default_indices = {
                    0, 1, 3,  3, 1, 2,
                    4, 5, 6, 6, 7, 4
                };
        };
    }
};
