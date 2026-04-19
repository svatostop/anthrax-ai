module;
#include <cstdint>
#include "aai/gfx/vk/backend/vk_defines.h"

export module aai.gfx.vk.frames.sync;

export {
   namespace vk {
       class synchronization {
           public:
                void init(VkDevice dev);
                
                void sync_frames(VkDevice dev, VkSwapchainKHR swapchain);
                bool is_swapchain_index_valid() const { return swapchain_index > -1 && swapchain_index < MAX_FRAMES; }
                uint32_t  get_swapchain_index() const { return swapchain_index; }
                VkFence* get_upload_fence() { return &upload_fence; }
           private:
                VkSemaphore present_sema[MAX_FRAMES];
                VkSemaphore render_sema[MAX_FRAMES];
                VkFence render_fence[MAX_FRAMES];

                VkFence upload_fence;

                uint32_t swapchain_index = 0;
        };
   }
};
