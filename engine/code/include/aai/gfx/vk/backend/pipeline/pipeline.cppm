module;
#include "aai/gfx/vk/backend/vk_defines.h"

export module aai.gfx.vk.pipeline;
import glm;
export import aai.gfx.materials;
export import aai.gfx.attachment_ref;
export import aai.gfx.assets;
import std;
export {
    namespace vk {
        class pipeline {
            public:
                void create_material(VkDevice dev, mat::materials& m, const rt::attachment_ref::info& attachments);
                
                void set_layout(VkDescriptorSetLayout l) { bindless_texture_layout = l; }
                
                struct vertex_desc {
                    std::vector<VkVertexInputBindingDescription> bindings;
                    std::vector<VkVertexInputAttributeDescription> attributes;
                    VkPipelineVertexInputStateCreateFlags flags = 0;
                };

                struct vertex {
                    glm::vec4 position = glm::vec4(0);
                    glm::vec3 normal = glm::vec3(0);
                    glm::vec3 color = glm::vec3(0,0,0);
                    glm::vec2 uv = glm::vec2(0,0);
                    float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f};
                    int boneID[4] = { -1, -1, -1, -1};
                };
                
                struct push_range {
                    uint32_t gpu_address = 0;
                };
            private:
                VkPipelineLayoutCreateInfo pipeline_layout_create_info();
                VkPipeline pipe;
                VkPipelineLayout pipe_layout;
                VkDescriptorSetLayout bindless_texture_layout;
                assets::base<std::string> shader_mng;
        };
    }
};
