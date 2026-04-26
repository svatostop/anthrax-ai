module;
#include "aai/gfx/vk/backend/vk_defines.h"
#include <cstdint>

export module aai.gfx.attachments;
import aai.gfx.vk.rt;
import aai.gfx.vk.device;
import std;
export {
    namespace rt {
        namespace attachments {
            enum rule {
                ATTACHMENT_RULE_DONT_CARE = 1 << 0,
                ATTACHMENT_RULE_LOAD  = 1 << 1,
                ATTACHMENT_RULE_CLEAR = 1 << 2,
            };
            enum class name {
                ONE_QUAD = 0,
            }; 
            enum class val {
                MAIN_COLOR = 0,
                DEPTH,
                SIZE
            };
            struct type {
                val v;
                VkFormat format;
            };
            struct ref {
                uint32_t color_count = 0;
                uint32_t depth_count = 0;
                std::vector<type> color_types;
                type depth_types;
            };
            std::map<name, ref> refs;
            rt::render_target* rts[static_cast<int>(val::SIZE)]; 
            void fill() {
                ref r;
                r.color_types.push_back({ val::MAIN_COLOR, VK_FORMAT_R8G8B8A8_UNORM });
                r.color_count = r.color_types.size();
                refs[name::ONE_QUAD] = r;
            }
            void create(const vk::device::handlers& dev) {
                rts[static_cast<int>(val::MAIN_COLOR)] = new render_target(static_cast<int>(val::MAIN_COLOR));
                rts[static_cast<int>(val::MAIN_COLOR)]->set_format(VK_FORMAT_R8G8B8A8_UNORM);
                rts[static_cast<int>(val::MAIN_COLOR)]->set_dimensions({800, 600});
                rts[static_cast<int>(val::MAIN_COLOR)]->set_sampler(true);
                rts[static_cast<int>(val::MAIN_COLOR)]->create(dev);
            }
            const ref& get_ref(name d)  { return refs[d]; }
            rt::render_target* get_rt(val v)  { return rts[static_cast<int>(v)]; }
        }
    }
};

