module;
#include <cstdint>

export module aai.gfx.vk.attachment_ref;
import std;
export {
    namespace rt {
        namespace attachment_ref {
            enum value {
                MAIN_COLOR = 0,
                DEPTH,
            };
            struct info {
                uint32_t color_count = 0;
                uint32_t depth_count = 0;
                std::vector<value> color_types;
                std::vector<value> depth_types;
            };
        }
    }
};

