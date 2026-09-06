export module aai.gfx.materials.types;

export import aai.gfx.vk.rt;
export import aai.gfx.vk.rt.helper;
import std;
import glm;
// todo improve macros
export {
    namespace mat {
        namespace color_blends {
#define COLOR_BLENDS_LOOKUP \
            X(SRC_ALPHA, "src_alpha") \
            X(ONE_MINUS_SRC_ALPHA, "one_minus_src_alpha") \
            X(BLEND_SIZE, "rts size") \

#define X(element, name) element,
            typedef enum {
                COLOR_BLENDS_LOOKUP
            } val;
#undef X
            const char* get_value(const val id)
            {
                const char* retval;
#define X(element, name) if (id == element) { retval = name; } else
                COLOR_BLENDS_LOOKUP
#undef X
                {
                    retval = "undef";
                }
                return retval;
            }
            val get_key(const char* id)
            {
                val retval;
#define X(element, name) if (id == name) { retval = element; } else
                COLOR_BLENDS_LOOKUP
#undef X
                {
                    retval = BLEND_SIZE;
                }
                return retval;
            }
            }
            namespace color_op {
#define COLOR_OP_LOOKUP \
            X(ADD, "add") \
            X(COLOR_OP_SIZE, "color op size") \

#define X(element, name) element,
            typedef enum {
                COLOR_OP_LOOKUP
            } val;
#undef X
            const char* get_value(const val id)
            {
                const char* retval;
#define X(element, name) if (id == element) { retval = name; } else
                COLOR_OP_LOOKUP
#undef X
                {
                    retval = "undef";
                }
                return retval;
            }
            val get_key(const char* id)
            {
                val retval;
#define X(element, name) if (id == name) { retval = element; } else
                COLOR_OP_LOOKUP
#undef X
                {
                    retval = COLOR_OP_SIZE;
                }
                return retval;
            }
            }
        namespace polygon {
#define POLYGON_LOOKUP \
            X(LINE, "line") \
            X(FILL, "fill") \
            X(POINT, "point") \
            X(SIZE, "polygon size") \

#define X(element, name) element,
            typedef enum {
                POLYGON_LOOKUP
            } val;
#undef X
            const char* get_value(const val id)
            {
                const char* retval;
#define X(element, name) if (id == element) { retval = name; } else
                POLYGON_LOOKUP
#undef X
                {
                    retval = "undef";
                }
                return retval;
            }
            val get_key(const char* id)
            {
                val retval;
#define X(element, name) if (id == name) { retval = element; } else
                POLYGON_LOOKUP
#undef X
                {
                    retval = SIZE;
                }
                return retval;
            }
            }
        namespace cull {
#define CULL_LOOKUP \
            X(FRONT, "front") \
            X(BACK, "back") \
            X(NONE, "none") \
            X(SIZE, "cull size") \

#define X(element, name) element,
            typedef enum {
                CULL_LOOKUP
            } val;
#undef X
            const char* get_value(const val id)
            {
                const char* retval;
#define X(element, name) if (id == element) { retval = name; } else
                CULL_LOOKUP
#undef X
                {
                    retval = "undef";
                }
                return retval;
            }
            val get_key(const char* id)
            {
                val retval;
#define X(element, name) if (id == name) { retval = element; } else
                CULL_LOOKUP
#undef X
                {
                    retval = SIZE;
                }
                return retval;
            }
            }
        namespace face {
#define FACE_LOOKUP \
            X(CC, "cc") \
            X(CCW, "ccw") \
            X(SIZE, "face size") \

#define X(element, name) element,
            typedef enum {
                FACE_LOOKUP
            } val;
#undef X
            const char* get_value(const val id)
            {
                const char* retval;
#define X(element, name) if (id == element) { retval = name; } else
                FACE_LOOKUP
#undef X
                {
                    retval = "undef";
                }
                return retval;
            }
            val get_key(const char* id)
            {
                val retval;
#define X(element, name) if (id == name) { retval = element; } else
                FACE_LOOKUP
#undef X
                {
                    retval = SIZE;
                }
                return retval;
            }
            }

        namespace depth_op {
#define DEPTH_OP_LOOKUP \
            X(DEPTH_OP_LESS_OR_EQUAL, "less_or_equal") \
            X(DEPTH_OP_GREATER, "greater") \
            X(SIZE, "depth size") \

#define X(element, name) element,
            typedef enum {
                DEPTH_OP_LOOKUP
            } val;
#undef X
            const char* get_value(const val id)
            {
                const char* retval;
#define X(element, name) if (id == element) { retval = name; } else
                DEPTH_OP_LOOKUP
#undef X
                {
                    retval = "undef";
                }
                return retval;
            }
            val get_key(const char* id)
            {
                val retval;
#define X(element, name) if (id == name) { retval = element; } else
                DEPTH_OP_LOOKUP
#undef X
                {
                    retval = SIZE;
                }
                return retval;
            }
            }
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
            mat::color_blends::val src_color_val = color_blends::val::BLEND_SIZE;
            mat::color_blends::val dst_color_val = color_blends::val::BLEND_SIZE;
            mat::color_blends::val src_alpha_val = color_blends::val::BLEND_SIZE;
            mat::color_blends::val dst_alpha_val = color_blends::val::BLEND_SIZE;
            mat::color_op::val c_op = color_op::val::COLOR_OP_SIZE; 
            mat::color_op::val a_op = color_op::val::COLOR_OP_SIZE; 
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
