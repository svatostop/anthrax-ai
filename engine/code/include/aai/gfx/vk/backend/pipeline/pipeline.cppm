module;
#include "aai/gfx/vk/backend/vk_defines.h"

export module aai.gfx.vk.pipeline;
import glm;
export import aai.gfx.materials;
export import aai.gfx.vk.rt;
export import aai.gfx.assets;
import std;
export {
    namespace vk {
        class pipeline {
            public:
                void create_material(VkDevice dev, mat::materials& m);
                
                void set_layout(VkDescriptorSetLayout l) { bindless_texture_layout = l; }
                
                struct vertex_desc {
                    std::vector<VkVertexInputBindingDescription> bindings;
                    std::vector<VkVertexInputAttributeDescription> attributes;
                    VkPipelineVertexInputStateCreateFlags flags = 0;
                };

                struct push_range {
                    VkDeviceAddress gpu_address = 0;
                    uint32_t texture_id = 0;
                };
            private:
                VkPipeline pipe;
                VkPipelineLayout pipe_layout;
                VkDescriptorSetLayout bindless_texture_layout;
                VkPipelineVertexInputStateCreateInfo vertex_input_info;
                VkPipelineViewportStateCreateInfo viewport_state;
                VkPipelineColorBlendStateCreateInfo color_blend_state; 
                VkViewport viewport;
                VkRect2D scissor;
                assets::base<std::string> shader_mng;
        };
    }
};
