module;
#include <vulkan/vulkan_core.h>
#include "aai/gfx/vk/model/model_types.h"

export module aai.keeper.types;
import std;
export {
    namespace keeper {
        struct back_datas {
            std::shared_ptr<model::skin_groups> skin_group;
            uint32_t model_id = 0;
            uint32_t texture_id = 0;
            uint32_t material_id = 0;
            std::string tag;
        };
 
    }
};
