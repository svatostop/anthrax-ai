module;
#include <cstdint>
#include "aai/gfx/vk/backend/vk_defines.h"

export module aai.gfx.vk.frames.sync;

export {
   namespace vk {
       class synchronization {
           public:
                void init(VkDevice dev);
                
                void sync_frames(VkDevice dev, VkSwapchainKHR swapchain, uint32_t frame_index);
                bool is_swapchain_index_valid() const {  return swapchain_index >= 0 && swapchain_index < MAX_FRAMES + 1; }
                uint32_t  get_swapchain_index() const { return swapchain_index; }
                uint32_t*  get_swapchain_index_ptr() { return &swapchain_index; }
                VkFence* get_upload_fence() { return &upload_fence; }

                VkSemaphore get_wait_sema(uint32_t frame_index) { return present_sema[frame_index]; }
                VkSemaphore get_render_sema(uint32_t frame_index) { return render_sema[frame_index]; }
                VkFence get_render_fence(uint32_t frame_index) { return render_fence[frame_index]; }
           private:
                VkSemaphore present_sema[MAX_FRAMES];
                VkSemaphore render_sema[MAX_FRAMES];
                VkFence render_fence[MAX_FRAMES];

                VkFence upload_fence;

                uint32_t swapchain_index = 0;
        };
   }
};
