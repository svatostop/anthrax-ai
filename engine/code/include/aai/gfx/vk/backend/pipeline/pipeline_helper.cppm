module;
#include "aai/gfx/vk/backend/vk_defines.h"
#include <shaderc/shaderc.h>

export module aai.gfx.vk.pipeline.helper;
import aai.gfx.materials;
import aai.utils;

export {
    namespace vk {
        namespace convert {
            VkPolygonMode polygon(mat::polygon_val val) {
                switch (val) {
                    case mat::MODE_LINE:
                        return VK_POLYGON_MODE_LINE;
                    case mat::MODE_FILL:
                        return VK_POLYGON_MODE_FILL;
                    case mat::MODE_POINT:
                        return VK_POLYGON_MODE_POINT;
                    default:
                    case mat::MODE_SIZE:
                        utils::ASSERT(true, "polygon_val not set!");
                        return VK_POLYGON_MODE_FILL;
                }
            }
            VkCullModeFlagBits cull(mat::cull_val val) {
                switch (val) {
                    case mat::CULL_FRONT:
                        return VK_CULL_MODE_FRONT_BIT;
                    case mat::CULL_BACK:
                        return VK_CULL_MODE_BACK_BIT;
                    default:
                    case mat::CULL_SIZE:
                        utils::ASSERT(true, "cull_val not set!");
                        return VK_CULL_MODE_NONE;
                }
            }
            VkFrontFace face(mat::face_val val) {
                switch (val) {
                    case mat::CC:
                        return VK_FRONT_FACE_CLOCKWISE;
                    case mat::CCW:
                        return VK_FRONT_FACE_COUNTER_CLOCKWISE;
                    default:
                    case mat::FACE_SIZE:
                        utils::ASSERT(true, "face_val not set!");
                        return VK_FRONT_FACE_COUNTER_CLOCKWISE;
                }
            }
            VkBlendFactor blend_factor(mat::color_blend_val val) {
                switch (val) {
                    case mat::SRC_ALPHA:
                        return VK_BLEND_FACTOR_SRC_ALPHA;
                    case mat::ONE_MINUS_SRC_ALPHA:
                        return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                    default:
                    case mat::BLEND_SIZE:
                        utils::ASSERT(true, "color_blend_val not set!");
                        return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                }
            }
            VkBlendOp blend_op(mat::color_op val) {
                switch (val) {
                    case mat::COLOR_OP_ADD:
                        return VK_BLEND_OP_ADD;
                    default:
                    case mat::COLOR_OP_SIZE:
                        utils::ASSERT(true, "color_op not set!");
                        return VK_BLEND_OP_ADD;
                }
            }
            VkCompareOp depth_cmp(mat::depth_op val) {
                switch (val) {
                    case mat::DEPTH_OP_LESS_OR_EQUAL:
                        return VK_COMPARE_OP_LESS_OR_EQUAL;
                    default:
                    case mat::DEPTH_OP_SIZE:
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

