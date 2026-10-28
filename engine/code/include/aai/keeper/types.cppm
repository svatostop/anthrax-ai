module;
#include <vulkan/vulkan_core.h>

export module aai.keeper.types;
import std;
export {
    namespace keeper {
        struct back_datas {
            std::vector<VkDeviceAddress> skin_gpu_addresses;
            uint32_t model_id = 0;
            uint32_t texture_id = 0;
            uint32_t material_id = 0;
            std::string tag;
        };
 
    }
};
