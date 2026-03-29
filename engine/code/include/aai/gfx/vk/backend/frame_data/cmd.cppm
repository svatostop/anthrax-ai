module;
#include <cstdint>
#include "aai/gfx/vk/backend/vk_defines.h"

export module aai.gfx.vk.frames.cmd;

export {
   namespace vk {
       class command_buffer {
           public:
                void init(VkDevice dev, const uint32_t graphics_index);

                VkCommandBuffer get_upload_cmd() { return upload_cmd; }
                VkCommandPool get_upload_cmd_pool() { return upload_cmd_pool; }
           private:
                VkCommandPool main_cmd_pool[MAX_FRAMES];
                VkCommandBuffer main_cmd[MAX_FRAMES];
                
                VkCommandPool upload_cmd_pool;
                VkCommandBuffer upload_cmd;
        };
   }
};
