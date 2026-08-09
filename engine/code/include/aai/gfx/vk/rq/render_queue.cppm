module;
#include <cstdint>
#include <memory>

export module aai.gfx.vk.rq;
import aai.gfx.materials;
import aai.gfx.vk.model;
import std;
import glm;
export {
    namespace vk {
        namespace rq {
            struct data {
                std::string tag;
                std::shared_ptr<mat::data> material_handle = nullptr;
                uint32_t texture_id = 0;
                std::shared_ptr<model::base> mesh_handle = nullptr;
                glm::mat4 model_matrix = glm::mat4(1);
            };
        }
    }
};

