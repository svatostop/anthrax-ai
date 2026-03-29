module;
#include <cstdint>
#include "aai/gfx/vk/backend/vk_defines.h"

export module aai.gfx.vk.frames;

export import aai.gfx.vk.frames.cmd;
export import aai.gfx.vk.frames.sync;
export {
   namespace vk {

        namespace submit_helper {

        }

       class frames {
           public:
                void init(VkDevice dev, const uint32_t graphics_index);
                
                VkCommandBuffer get_upload_cmd() { return cmd.get_upload_cmd(); }
                VkFence* get_upload_fence() { return sync.get_upload_fence(); }
                VkCommandPool get_upload_cmd_pool() { return cmd.get_upload_cmd_pool(); }

                VkCommandBufferBeginInfo cmd_begin_info(VkCommandBufferUsageFlags flags);
                VkSubmitInfo submit_info(VkCommandBuffer* cmd);
                
                private:
                command_buffer cmd;
                synchronization sync;
        };
   }
};
