#pragma once

#include "anthraxAI/utils/defines.h"
#include "anthraxAI/utils/mathdefines.h"
#include "anthraxAI/gfx/vkdefines.h"
#include "anthraxAI/gfx/renderhelpers.h"
#include <vulkan/vulkan_core.h>

namespace Gfx
{
    struct RenderingAttachmentInfo {
        Gfx::AttachmentRules Rules;
        VkImage Image;
        VkImageView ImageView;
        VkImageLayout Layout = VK_IMAGE_LAYOUT_UNDEFINED;
        bool IsDepth;
    };

    class CommandBuffer
    {
        public:
            VkCommandBuffer GetCmd() { return cmd; }
            void SetCmd(VkCommandBuffer c) { cmd = c; }

            VkImageSubresourceRange GetSubresourceMainRange() { return range; }

            VkCommandBufferBeginInfo InfoCmd(VkCommandBufferUsageFlags flags);
            void BeginCmd(VkCommandBufferBeginInfo cmdinfo);
            void EndCmd();

            VkSubmitInfo SubmitInfo(VkPipelineStageFlags waitstage, VkSemaphore* presentsem, VkSemaphore* rendersem);
            void Submit(VkQueue queue, VkSubmitInfo subinfo, VkFence* fence);

            VkPresentInfoKHR PresentInfo(VkSwapchainKHR* swapchain, VkSemaphore* rendersem, uint32_t* swapchind);
            VkResult Present(VkQueue queue, VkPresentInfoKHR prinfo);

            VkRenderingAttachmentInfoKHR GetAttachmentInfo(VkImageView imageview, bool iscolor, Gfx::AttachmentRules loadop);
            const VkRenderingInfoKHR GetRenderingInfo(std::vector<RenderingAttachmentInfo>& attachmentinfo, std::vector<VkRenderingAttachmentInfoKHR>& colors, VkRenderingAttachmentInfoKHR& depthinfo, Vector2<int> extents, bool multithreaded = false);

            void MemoryBarrier(VkImage image, VkImageLayout oldlayout, VkImageLayout newlayout, VkImageSubresourceRange range);
            void MemoryBarrier(VkAccessFlags srcaccess, VkAccessFlags dstaccess, VkPipelineStageFlags srcstage, VkPipelineStageFlags dststage);
            void MemoryBarrier(VkBuffer buffer,  VkDeviceSize size,VkAccessFlags srcaccess, VkAccessFlags dstaccess, VkPipelineStageFlags srcstage, VkPipelineStageFlags dststage);
            void CopyImage(VkImage src, Vector2<int> srcsize, VkImageLayout srcoldlayout, VkImageLayout srcnewlayout, VkImage dst, Vector2<int> dstsize, VkImageLayout dstoldlayout, VkImageLayout dstnewlayout);

            void SetRange(VkImageSubresourceRange r) { range = r;}
        private:
        	VkImageSubresourceRange range{};

            VkClearValue clearimg[3];
            VkCommandBuffer cmd;
    };
}
