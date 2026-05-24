module;
#include <cstdint>
#include "aai/gfx/vk/backend/vk_defines.h"

export module aai.gfx.vk.frames.sync;

import std;
export {
   namespace vk {
       class synchronization {
           public:
                void init(VkDevice dev, const int sw_size);
                
                void sync_frames(VkDevice dev, VkSwapchainKHR swapchain, uint32_t frame_index);
                bool is_swapchain_index_valid() const {  return swapchain_index >= 0 && swapchain_index < swapchain_size; }
                uint32_t  get_swapchain_index() const { return swapchain_index; }
                uint32_t*  get_swapchain_index_ptr() { return &swapchain_index; }
                VkFence* get_upload_fence() { return &upload_fence; }

                VkSemaphore get_wait_sema(int frame_index) { return present_sema[frame_index]; }
                VkSemaphore get_render_sema() { return render_sema[swapchain_index]; }
                
                struct timeline_sema {
                    VkSemaphore handle = VK_NULL_HANDLE;
                    uint64_t value = 0;
                };
                VkSemaphore get_timeline() { return timeline.handle; }
                uint64_t get_timeline_value() { return timeline.value; }
                void set_timeline_value(uint64_t v) { timeline.value = v; }

                void wait_timeline(VkDevice dev);
           private:
                std::vector<VkSemaphore> present_sema;
                std::vector<VkSemaphore> render_sema;

                VkFence upload_fence;

                timeline_sema timeline;
            
                uint32_t swapchain_size = 0;
                uint32_t swapchain_index = 0;
                bool first_run = true;
                PFN_vkWaitSemaphoresKHR vkWaitSemaphoresKHR{ VK_NULL_HANDLE };
        };
   }
};
