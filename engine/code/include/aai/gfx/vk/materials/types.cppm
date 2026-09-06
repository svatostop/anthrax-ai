module;
#include "aai/utils/lookup_table.h"

export module aai.gfx.materials.types;

export import aai.gfx.vk.rt;
export import aai.gfx.vk.rt.helper;
import aai.utils;
import std;
import glm;
    
export {
    namespace mat {
        namespace color_blends {
#define COLOR_BLENDS_LOOKUP(X) \
            X(SRC_ALPHA, "src_alpha") \
            X(ONE_MINUS_SRC_ALPHA, "one_minus_src_alpha") \
            X(SIZE, "rts size")
DECLARE_LOOKUP_TABLE(COLOR_BLENDS_LOOKUP, val)
        }
        namespace color_op {
#define COLOR_OP_LOOKUP(X) \
            X(ADD, "add") \
            X(SIZE, "color op size") 
DECLARE_LOOKUP_TABLE(COLOR_OP_LOOKUP, val)
        }
        namespace polygon {
#define POLYGON_LOOKUP(X) \
            X(LINE, "line") \
            X(FILL, "fill") \
            X(POINT, "point") \
            X(SIZE, "polygon size")
DECLARE_LOOKUP_TABLE(POLYGON_LOOKUP, val)
        }
        namespace cull {
#define CULL_LOOKUP(X) \
            X(FRONT, "front") \
            X(BACK, "back") \
            X(NONE, "none") \
            X(SIZE, "cull size")
DECLARE_LOOKUP_TABLE(CULL_LOOKUP, val)
        }
        namespace face {
#define FACE_LOOKUP(X) \
            X(CC, "cc") \
            X(CCW, "ccw") \
            X(SIZE, "face size") 
DECLARE_LOOKUP_TABLE(FACE_LOOKUP, val)
        }
        namespace depth_op {
#define DEPTH_OP_LOOKUP(X) \
            X(DEPTH_OP_LESS_OR_EQUAL, "less_or_equal") \
            X(DEPTH_OP_GREATER, "greater") \
            X(SIZE, "depth size") 
DECLARE_LOOKUP_TABLE(DEPTH_OP_LOOKUP, val)
        }
        enum shader_type {
            SHADER_FRAG = 0,
            SHADER_VERT,
            SHADER_COMPUTE,
        };
        struct shader_module {
            shader_type t;
            std::string path;
        };
        struct color_blend_helper {
            mat::color_blends::val src_color_val = color_blends::val::SIZE;
            mat::color_blends::val dst_color_val = color_blends::val::SIZE;
            mat::color_blends::val src_alpha_val = color_blends::val::SIZE;
            mat::color_blends::val dst_alpha_val = color_blends::val::SIZE;
            mat::color_op::val c_op = color_op::val::SIZE; 
            mat::color_op::val a_op = color_op::val::SIZE; 
            bool alpha_blend = false;
        };
        struct rasterizer_helper {
            polygon::val polygon = polygon::SIZE;
            cull::val cull = cull::SIZE;
            face::val face = face::SIZE;
        };
        struct depth_helper {
            depth_op::val d_op = depth_op::val::SIZE;
            bool depth_write = false;
            bool depth_test = false;
        };
        struct info_helper {
            std::string name;
            rt::base::ref rt_ref;
            rt::name::val rt_ref_val;
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

        using material_infos_map = std::map<std::string, info_helper>; 


    }

};
