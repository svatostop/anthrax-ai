module;
#include "aai/gfx/vk/backend/vk_defines.h"
#include <shaderc/shaderc.h>

export module aai.gfx.vk.pipeline.helper;
import aai.gfx.materials;
import aai.utils;

export {
    namespace vk {
        namespace convert {
            VkPolygonMode polygon(mat::polygon::val val) {
                switch (val) {
                    case mat::polygon::val::LINE:
                        return VK_POLYGON_MODE_LINE;
                    case mat::polygon::val::FILL:
                        return VK_POLYGON_MODE_FILL;
                    case mat::polygon::val::POINT:
                        return VK_POLYGON_MODE_POINT;
                    default:
                    case mat::polygon::val::SIZE:
                        utils::ASSERT(true, "polygon_val not set!");
                        return VK_POLYGON_MODE_FILL;
                }
            }
            VkCullModeFlagBits cull(mat::cull::val val) {
                switch (val) {
                    case mat::cull::val::FRONT:
                        return VK_CULL_MODE_FRONT_BIT;
                    case mat::cull::val::BACK:
                        return VK_CULL_MODE_BACK_BIT;
                    case mat::cull::val::NONE:
                        return VK_CULL_MODE_NONE;
                    default:
                    case mat::cull::val::SIZE:
                        utils::ASSERT(true, "cull_val not set!");
                        return VK_CULL_MODE_NONE;
                }
            }
            VkFrontFace face(mat::face::val val) {
                switch (val) {
                    case mat::face::val::CC:
                        return VK_FRONT_FACE_CLOCKWISE;
                    case mat::face::val::CCW:
                        return VK_FRONT_FACE_COUNTER_CLOCKWISE;
                    default:
                    case mat::face::val::SIZE:
                        utils::ASSERT(true, "face_val not set!");
                        return VK_FRONT_FACE_COUNTER_CLOCKWISE;
                }
            }
            VkBlendFactor blend_factor(mat::color_blends::val val) {
                switch (val) {
                    case mat::color_blends::val::SRC_ALPHA:
                        return VK_BLEND_FACTOR_SRC_ALPHA;
                    case mat::color_blends::val::ONE_MINUS_SRC_ALPHA:
                        return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                    default:
                    case mat::color_blends::val::BLEND_SIZE:
                        utils::ASSERT(true, "mat::color_blends::val not set!");
                        return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                }
            }
            VkBlendOp blend_op(mat::color_op::val val) {
                switch (val) {
                    case mat::color_op::val::ADD:
                        return VK_BLEND_OP_ADD;
                    default:
                    case mat::color_op::val::COLOR_OP_SIZE:
                        utils::ASSERT(true, "mat::color_op::val not set!");
                        return VK_BLEND_OP_ADD;
                }
            }
            VkCompareOp depth_cmp(mat::depth_op::val val) {
                switch (val) {
                    case mat::depth_op::DEPTH_OP_LESS_OR_EQUAL:
                        return VK_COMPARE_OP_LESS_OR_EQUAL;
                    case mat::depth_op::DEPTH_OP_GREATER:
                        return VK_COMPARE_OP_GREATER;
                    default:
                    case mat::depth_op::SIZE:
                        utils::ASSERT(true, "depth_op not set!");
                        return VK_COMPARE_OP_LESS_OR_EQUAL;
                }
            }
            VkShaderStageFlagBits shader_type_vk(mat::shader_type t) {
                switch (t) {
                    case mat::SHADER_FRAG:
                        return VK_SHADER_STAGE_FRAGMENT_BIT;
                    case mat::SHADER_VERT:
                        return VK_SHADER_STAGE_VERTEX_BIT;
                    default:
                    case mat::SHADER_COMPUTE:
                        return VK_SHADER_STAGE_COMPUTE_BIT;
                }
            }
            shaderc_shader_kind shader_type_sc(mat::shader_type t) {
                switch (t) {
                    case mat::SHADER_FRAG:
                        return shaderc_glsl_fragment_shader;
                    case mat::SHADER_VERT:
                        return shaderc_glsl_vertex_shader;
                    default:
                    case mat::SHADER_COMPUTE:
                        return shaderc_glsl_compute_shader;
                }

            }
        }
    }
};

