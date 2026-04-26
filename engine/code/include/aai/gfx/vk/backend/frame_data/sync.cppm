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

                VkSemaphore get_wait_sema() { return present_sema[swapchain_index]; }
                VkSemaphore get_render_sema() { return render_sema[swapchain_index]; }
                VkFence get_render_fence() { return render_fence[swapchain_index]; }
           private:
                VkSemaphore present_sema[MAX_FRAMES];
                VkSemaphore render_sema[MAX_FRAMES];
                VkFence render_fence[MAX_FRAMES];

                VkFence upload_fence;

                uint32_t swapchain_index = 0;
        };
   }
};
