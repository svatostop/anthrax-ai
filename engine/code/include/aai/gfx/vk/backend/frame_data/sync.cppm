module;
#include <cstdint>
#include "aai/gfx/vk/backend/vk_defines.h"

export module aai.gfx.vk.frames.sync;

export {
   namespace vk {
       class synchronization {
           public:
                void init(VkDevice dev);

                VkFence* get_upload_fence() { return &upload_fence; }
           private:
                VkSemaphore present_sema[MAX_FRAMES + 1];
                VkSemaphore render_sema[MAX_FRAMES + 1];
                VkFence render_fence[MAX_FRAMES + 1];

                VkFence upload_fence;

                uint32_t swapchain_index = 0;
        };
   }
};
