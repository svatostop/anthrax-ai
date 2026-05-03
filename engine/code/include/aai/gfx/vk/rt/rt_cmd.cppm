module;
#include "aai/gfx/vk/backend/vk_defines.h"

export module aai.gfx.vk.rt.cmd;
import aai.utils;
import glm;
export {
    namespace rt {
        namespace cmd {
            VkAccessFlags get_access_flags(VkImageLayout layout)
            {
            	switch (layout)
            	{
            		case VK_IMAGE_LAYOUT_UNDEFINED:
            		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            			return 0;
            		case VK_IMAGE_LAYOUT_PREINITIALIZED:
            			return VK_ACCESS_HOST_WRITE_BIT;
            		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            			return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
            			return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            		case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
            			return VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;
            		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            			return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
            		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            			return VK_ACCESS_TRANSFER_READ_BIT;
            		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            			return VK_ACCESS_TRANSFER_WRITE_BIT;
            		case VK_IMAGE_LAYOUT_GENERAL:
            		 	utils::ASSERT(false, "Don't use VK_IMAGE_LAYOUT_GENERAL!");
            			return 0;
            		default:
                        utils::ASSERT(false, "weird image layout!");
            			return 0;
            	}
            }
            
            VkPipelineStageFlags get_pipeline_stage_flags(VkImageLayout layout)
            {
            	switch (layout)
            	{
            		case VK_IMAGE_LAYOUT_UNDEFINED:
            			return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            		case VK_IMAGE_LAYOUT_PREINITIALIZED:
            			return VK_PIPELINE_STAGE_HOST_BIT;
            		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            			return VK_PIPELINE_STAGE_TRANSFER_BIT;
            		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            			return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
            			return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            		case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
            			return VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
            		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            			return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            			return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            		case VK_IMAGE_LAYOUT_GENERAL:
            			utils::ASSERT(false, "Don't use VK_IMAGE_LAYOUT_GENERAL!");
            			return 0;
            		default:
            			return 0;
            	}
            }
            void memory_barrier(VkCommandBuffer cmd, VkImage image, VkImageLayout oldlayout, VkImageLayout newlayout)
            {
                VkImageSubresourceRange range{};
                range.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            	range.baseMipLevel   = 0;
            	range.levelCount     = VK_REMAINING_MIP_LEVELS;
            	range.baseArrayLayer = 0;
            	range.layerCount     = VK_REMAINING_ARRAY_LAYERS;

            	VkPipelineStageFlags srcstagemask  = get_pipeline_stage_flags(oldlayout);
            	VkPipelineStageFlags dststagemask  = get_pipeline_stage_flags(newlayout);
            	VkAccessFlags srcaccessmask = get_access_flags(oldlayout);
            	VkAccessFlags dstaccessmask = get_access_flags(newlayout);
            
                VkImageMemoryBarrier membarrier{};
            	membarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            	membarrier.srcAccessMask       = srcaccessmask;
            	membarrier.dstAccessMask       = dstaccessmask;
            	membarrier.oldLayout           = oldlayout;
            	membarrier.newLayout           = newlayout;
            	membarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            	membarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            	membarrier.image               = image;
            	membarrier.subresourceRange    = range;
            
            	vkCmdPipelineBarrier(cmd, srcstagemask, dststagemask, 0, 0, nullptr, 0, nullptr, 1, &membarrier);
            }
            void copy_image(VkCommandBuffer cmd, VkImage src_image, const glm::vec2& src_size, VkImageLayout src_old_layout, VkImageLayout src_new_layout, VkImage dst_image, const glm::vec2& dst_size, VkImageLayout dst_old_layout, VkImageLayout dst_new_layout)
            {
                VkImageSubresourceRange range{};
                range.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            	range.baseMipLevel   = 0;
            	range.levelCount     = VK_REMAINING_MIP_LEVELS;
            	range.baseArrayLayer = 0;
            	range.layerCount     = VK_REMAINING_ARRAY_LAYERS;

                memory_barrier(cmd, src_image, src_old_layout, src_new_layout);
            	memory_barrier(cmd, dst_image, dst_old_layout, dst_new_layout);
            
            	VkImageBlit blit;
            	blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            	blit.srcSubresource.baseArrayLayer = 0;
            	blit.srcSubresource.layerCount     = 1;
            	blit.srcSubresource.mipLevel       = 0;
            	blit.srcOffsets[0]                 = {0, 0, 0};
            	blit.srcOffsets[1]                 = {static_cast<int>(src_size.x), static_cast<int>(src_size.y), 1};
            
            	blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            	blit.dstSubresource.baseArrayLayer = 0;
            	blit.dstSubresource.layerCount     = 1;
            	blit.dstSubresource.mipLevel       = 0;
            	blit.dstOffsets[0]                 = {0, 0, 0};
            	blit.dstOffsets[1]                 = {static_cast<int>(dst_size.x), static_cast<int>(dst_size.y), 1};
            
            	vkCmdBlitImage(cmd,
            		src_image, src_new_layout,
            		dst_image, dst_new_layout,
            		1,
            		&blit,
            		VK_FILTER_NEAREST);
            }
        }
    }
};

