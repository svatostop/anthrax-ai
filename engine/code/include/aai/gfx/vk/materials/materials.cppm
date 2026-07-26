module;
#include "aai/gfx/vk/backend/vk_defines.h"
#include <vulkan/vulkan_core.h>

export module aai.gfx.materials;
import aai.gfx.vk.rt;
import aai.gfx.vk.device;
import glm;
import std;
export {
    namespace mat {
        struct data {               
            VkPipelineLayout pipeline_layout;
            VkPipeline pipeline;
            rt::base::ref attachment_ref;
            bool dynamic_viewport;

            void clean(VkDevice dev) {
                vkDestroyPipelineLayout(dev, pipeline_layout, nullptr);
                vkDestroyPipeline(dev, pipeline, nullptr);
            }
        };
        enum polygon_val {
            MODE_LINE = 0,
            MODE_FILL,
            MODE_POINT,
            MODE_SIZE
        };
        enum cull_val {
            CULL_FRONT = 0,
            CULL_BACK,
            CULL_NONE,
            CULL_SIZE
        };
        enum face_val {
            CC = 0,
            CCW,
            FACE_SIZE
        };
        enum color_blend_val {
            SRC_ALPHA = 0,
            ONE_MINUS_SRC_ALPHA,
            BLEND_SIZE
        };
        enum color_op {
            COLOR_OP_ADD = 0,
            COLOR_OP_SIZE
        };
        enum depth_op {
            DEPTH_OP_LESS_OR_EQUAL =0,
            DEPTH_OP_SIZE
        };
        enum shader_type {
            SHADER_FRAG = 0,
            SHADER_VERT,
            SHADER_COMPUTE
        };
        struct shader_module {
            shader_type t;
            std::string path;
        };
        struct color_blend_helper {
            color_blend_val src_color_val = BLEND_SIZE;
            color_blend_val dst_color_val = BLEND_SIZE;
            color_blend_val src_alpha_val = BLEND_SIZE;
            color_blend_val dst_alpha_val = BLEND_SIZE;
            color_op c_op = COLOR_OP_SIZE; 
            color_op a_op = COLOR_OP_SIZE; 
            bool alpha_blend = false;
        };
        struct rasterizer_helper {
            polygon_val polygon = MODE_SIZE;
            cull_val cull = CULL_SIZE;
            face_val face = FACE_SIZE;
        };
        struct depth_helper {
            depth_op d_op = DEPTH_OP_SIZE;
            bool depth_write = false;
            bool depth_test = false;
        };
        struct info_helper {
            std::string name;
            rt::base::ref rt_ref;
            glm::vec4 viewport = glm::vec4(0);
            glm::vec4 scissor = glm::vec4(0);
            bool dynamic_viewport = false;
            rasterizer_helper rasterizer;            
            color_blend_helper color_blend;
            depth_helper depth_stencil;
            std::vector<shader_module> shaders;
            bool multisampling = false;
            bool vertex_attributes = false;
            bool bind_texture = false;
            bool has_bones = false;
        };
 
        class materials {
            public:
                void add_material_info(info_helper d) { infos = d; } 
                void set_data(const std::string& n, VkPipeline pipe, VkPipelineLayout pipe_layout, const rt::base::ref& r, bool dynamic_viewport) {
                    std::shared_ptr<data> d(new data);
                    d->pipeline = pipe; 
                    d->pipeline_layout = pipe_layout; 
                    d->attachment_ref = r;
                    d->dynamic_viewport = dynamic_viewport;
                    mat_map[n] = d;
                }
                const info_helper& get_info() { return infos; }
                std::shared_ptr<data> get(const std::string& n) { 
                	auto it = mat_map.find(n);
                	if (it == mat_map.end()) {
                		return nullptr;
                	}
                	else {
                		return (*it).second;
                	}
                }

                void clean(const vk::device::handlers& dev) {
                    for (auto& m : mat_map) {
                        m.second->clean(dev.dev);
                    }
                }
            private:
                info_helper infos;
                std::map<std::string, std::shared_ptr<data>> mat_map; 
        };
    }
};

