module;
// #include <cstdint>
#include "aai/gfx/vk/backend/vk_defines.h"

export module aai.gfx.vk.frames;

export import aai.gfx.vk.frames.cmd;
export import aai.gfx.vk.frames.sync;
export import glm;
export import std;
export {
   namespace vk {

        namespace submit_helper {

        }

       class frames {
           public:
                void init(VkDevice dev, const uint32_t graphics_index);
                
                void submit(VkDevice dev, VkQueue queue, std::function<void(VkCommandBuffer cmd)>&& f);

                void sync_frames(VkDevice dev, VkSwapchainKHR swapchain);
                void prepare_for_present(VkImage src_image, const glm::vec2& src_size, VkImage sw_image, const glm::vec2& sw_size);
                void submit_and_present(VkQueue queue, VkSwapchainKHR swapchain);
                VkCommandBuffer get_upload_cmd() { return cmd.get_upload_cmd(); }
                VkFence* get_upload_fence() { return sync.get_upload_fence(); }
                VkCommandPool get_upload_cmd_pool() { return cmd.get_upload_cmd_pool(); }

                VkCommandBuffer get_cmd() { return cmd.get(frame_index); }

                VkCommandBufferBeginInfo cmd_begin_info(VkCommandBufferUsageFlags flags);
                VkSubmitInfo submit_info(VkCommandBuffer* cmd);
                
                uint32_t get_swapchain_index() const { return sync.get_swapchain_index(); }
                bool is_swapchain_index_valid() const { return sync.is_swapchain_index_valid(); }
                private:
                void inc_frame_ind() { frame_index = (frame_index + 1) % MAX_FRAMES; }

                command_buffer cmd;
                synchronization sync;
                uint32_t frame_index = 0;
        };
   }
};
