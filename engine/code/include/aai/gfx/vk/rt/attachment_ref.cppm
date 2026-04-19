module;
#include "aai/gfx/vk/backend/vk_defines.h"
#include <cstdint>

export module aai.gfx.attachment_ref;
import std;
export {
    namespace rt {
        namespace attachment_ref {
            enum class def {
                ONE_QUAD = 0,
            }; 
            enum class value {
                MAIN_COLOR = 0,
                DEPTH,
            };
            struct type {
                value val;
                VkFormat format;
            };
            struct info {
                uint32_t color_count = 0;
                uint32_t depth_count = 0;
                std::vector<type> color_types;
                type depth_types;
            };
            std::map<def, info> refs;
            void fill() {
                info r;
                r.color_types.push_back({ value::MAIN_COLOR, VK_FORMAT_R8G8B8A8_UNORM });
                r.color_count = r.color_types.size();
                refs[def::ONE_QUAD] = r;
            }
            const info& get(def d)  { return refs[d]; }
        }
    }
};

