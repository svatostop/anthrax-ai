module;
#include "aai/gfx/vk/backend/vk_defines.h"
#include <cstdint>

export module aai.gfx.vk.rt.helper;
import aai.gfx.vk.device;
import std;
export {
    namespace rt {
        namespace helper {
#define RT_LOOKUP \
            X(MAIN_COLOR, "main_color") \
            X(MAIN_DEPTH, "main_depth") \
            X(SIZE, "rts size") \

#define X(element, name) element,
            typedef enum {
                RT_LOOKUP
            } val;
#undef X
            const char* get_value(const val id)
            {
                const char* retval;
#define X(element, name) if (id == element) { retval = name; } else
                RT_LOOKUP
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
                RT_LOOKUP
#undef X
                {
                    retval = SIZE;
                }
                return retval;
            }

            enum rule {
                DONT_CARE = 1 << 0,
                LOAD  = 1 << 1,
                CLEAR = 1 << 2,
            };
        }
        namespace name {
#define RT_REF_LOOKUP \
            X(ONE_QUAD, "one_quad") \
            X(COLOR_WITH_DEPTH, "color_with_depth") \
            X(SIZE, "rt ref name size") \

#define X(element, name) element,
            typedef enum {
                RT_REF_LOOKUP
            } val;
#undef X
            const char* get_value(const val id)
            {
                const char* retval;
#define X(element, name) if (id == element) { retval = name; } else
                RT_REF_LOOKUP
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
                RT_REF_LOOKUP
#undef X
                {
                    retval = SIZE;
                }
                return retval;
            }

        }
    }
};

