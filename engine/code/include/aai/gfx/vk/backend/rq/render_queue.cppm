module;
#include <cstdint>

export module aai.gfx.vk.rq;
import aai.gfx.materials;
import std;
export {
    namespace vk {
        namespace rq {
            struct data {
                std::string tag;
                mat::data* material_handle = nullptr;
                uint32_t texture_id = 0;
            };
        }
    }
};

