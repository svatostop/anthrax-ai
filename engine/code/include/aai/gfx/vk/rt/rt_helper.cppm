module;
#include "aai/gfx/vk/backend/vk_defines.h"
#include <cstdint>

// #define RT \
//     X(RT_MAIN_COLOR, "main_color") \
//     X(RT_MAIN_DEBUG, "main_debug") \
//     X(RT_DEPTH, "depth") \
//     X(RT_ALBEDO, "albedo") \
//     X(RT_NORMAL, "normal") \
//     X(RT_POSITION, "position") \
//     X(RT_GBUFFER_HELPER, "gbuffer_helper") \
//     X(RT_MASK, "mask") \
//     X(RT_SHADOWS, "shadows") \
//     X(RT_VISIBILITY, "visibility") \
//     X(RT_SIZE, "rts size") \
//
// #define X(element, name) element,
//     typedef enum {
//         RT
//     } RenderTargetsList;
// #undef X
//
//     static std::string GetValue(const RenderTargetsList id)
//     {
//         std::string retval;
// #define X(element, name) if (id == element) { retval = name; } else
//     RT
// #undef X
//         {
//             retval = "undef";
//         }
//         return retval;
//     }
//     static RenderTargetsList GetKey(const std::string& id)
//     {
//         RenderTargetsList retval;
// #define X(element, name) if (id == name) { retval = element; } else
//     RT
// #undef X
//         {
//             retval = RT_SIZE;
//         }
//         return retval;
//     }


export module aai.gfx.vk.rt.helper;
import aai.gfx.vk.device;
import std;
export {
    namespace rt {
        namespace helper {
             enum rule {
                ATTACHMENT_RULE_DONT_CARE = 1 << 0,
                ATTACHMENT_RULE_LOAD  = 1 << 1,
                ATTACHMENT_RULE_CLEAR = 1 << 2,
            };
            enum class val {
                MAIN_COLOR = 0,
                DEPTH,
                SIZE
            };
        }
    }
};

