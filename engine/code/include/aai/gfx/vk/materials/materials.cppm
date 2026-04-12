module;
#include "aai/gfx/vk/backend/vk_defines.h"

export module aai.gfx.materials;
import glm;
import std;
export {
    namespace mat {
        struct data {               
            VkPipelineLayout pipeline_layout;
            VkPipeline pipeline;
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
            glm::vec4 viewport = glm::vec4(0);
            glm::vec4 scissor = glm::vec4(0);
            rasterizer_helper rasterizer;            
            color_blend_helper color_blend;
            depth_helper depth_stencil;
            shader_module shaders;
            bool multisampling = false;
        };
 
        class materials {
            public:
                void add_material(info_helper d) {}
                
                void set_data(VkPipeline pipe, VkPipelineLayout pipe_layout) { m.pipeline = pipe; m.pipeline_layout = pipe_layout; }
                const info_helper& get_info() const { return info; }
            private:
                info_helper info;
                data m; 
        };
    }
};

