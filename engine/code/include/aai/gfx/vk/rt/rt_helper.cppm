module;
#include "aai/gfx/vk/backend/vk_defines.h"
#include "aai/utils/lookup_table.h"
#include <cstdint>

export module aai.gfx.vk.rt.helper;
import aai.gfx.vk.device;
import std;
export {
    namespace rt {
        namespace helper {
#define RT_LOOKUP(X) \
            X(MAIN_COLOR, "main_color") \
            X(MAIN_DEPTH, "main_depth") \
            X(SIZE, "rts size") 
DECLARE_LOOKUP_TABLE(RT_LOOKUP, val)

            enum rule {
                DONT_CARE = 1 << 0,
                LOAD  = 1 << 1,
                CLEAR = 1 << 2,
            };
        }
        namespace name {
#define RT_REF_LOOKUP(X) \
            X(ONE_QUAD, "one_quad") \
            X(COLOR_WITH_DEPTH, "color_with_depth") \
            X(SIZE, "rt ref name size") 
DECLARE_LOOKUP_TABLE(RT_REF_LOOKUP, val)
        }
    }
};

