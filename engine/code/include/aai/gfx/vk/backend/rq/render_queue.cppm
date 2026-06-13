module;
#include <cstdint>
#include <memory>

export module aai.gfx.vk.rq;
import aai.gfx.materials;
import aai.gfx.vk.model;
import std;
export {
    namespace vk {
        namespace rq {
            struct data {
                std::string tag;
                mat::data* material_handle = nullptr;
                uint32_t texture_id = 0;
                std::shared_ptr<model::base> mesh_handle = nullptr;
            };
        }
    }
};

