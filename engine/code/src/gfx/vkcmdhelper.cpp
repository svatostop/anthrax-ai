#include "anthraxAI/gfx/vkcmdhelper.h"
#include <vulkan/vulkan_core.h>

VkAccessFlags GetAccessFlags(VkImageLayout layout)
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
			ASSERT(false, "Don't use VK_IMAGE_LAYOUT_GENERAL!");
			return 0;
		default:
			assert(false);
			return 0;
	}
}

VkPipelineStageFlags GetPipelineStageFlags(VkImageLayout layout)
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
			ASSERT(false, "Don't use VK_IMAGE_LAYOUT_GENERAL!");
			return 0;
		default:
			return 0;
	}
}

VkCommandBufferBeginInfo Gfx::CommandBuffer::InfoCmd(VkCommandBufferUsageFlags flags)
{
    VkCommandBufferBeginInfo cmdbegininfo = {};
	cmdbegininfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdbegininfo.pNext = nullptr;

	cmdbegininfo.pInheritanceInfo = nullptr;
	cmdbegininfo.flags = flags;
    return cmdbegininfo;
}

VkSubmitInfo Gfx::CommandBuffer::SubmitInfo(VkPipelineStageFlags waitstage, VkSemaphore* presentsem, VkSemaphore* rendersem)
{
   	VkSubmitInfo submit = {};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.pNext = nullptr;
	VkPipelineStageFlags waitstage2 = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submit.pWaitDstStageMask = &waitstage2;
	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = presentsem;
	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = rendersem;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &cmd;

    return submit;
}

VkPresentInfoKHR Gfx::CommandBuffer::PresentInfo(VkSwapchainKHR* swapchain, VkSemaphore* rendersem, uint32_t* swapchind)
{
    VkPresentInfoKHR presentinfo = {};
	presentinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentinfo.pNext = nullptr;
	presentinfo.pSwapchains = swapchain;
	presentinfo.swapchainCount = 1;
	presentinfo.pWaitSemaphores = rendersem;
	presentinfo.waitSemaphoreCount = 1;
	presentinfo.pImageIndices = swapchind;

    return presentinfo;
}

VkResult Gfx::CommandBuffer::Present(VkQueue queue, VkPresentInfoKHR prinfo)
{
    return vkQueuePresentKHR(queue, &prinfo);
}

void Gfx::CommandBuffer::Submit(VkQueue queue, VkSubmitInfo subinfo, VkFence* fence)
{
	VK_ASSERT(vkQueueSubmit(queue, 1, &subinfo, *fence), "failed to submit queue!");
}

void Gfx::CommandBuffer::BeginCmd(VkCommandBufferBeginInfo cmdinfo)
{
	VK_ASSERT(vkBeginCommandBuffer(cmd, &cmdinfo), "failed to begin a command buffer!");
}

void Gfx::CommandBuffer::EndCmd()
{
	VK_ASSERT(vkEndCommandBuffer(cmd), "failed to end command buffer");
}

void Gfx::CommandBuffer::MemoryBarrier(VkBuffer buffer, VkDeviceSize size, VkAccessFlags srcaccess, VkAccessFlags dstaccess, VkPipelineStageFlags srcstage, VkPipelineStageFlags dststage)
{
    VkBufferMemoryBarrier buf_barrier{};
    buf_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    buf_barrier.srcAccessMask = srcaccess;
    buf_barrier.dstAccessMask = dstaccess;
    buf_barrier.buffer = buffer;
    buf_barrier.size = size;
    buf_barrier.offset = 0;

	vkCmdPipelineBarrier(cmd, srcstage, dststage, 0, 0, nullptr, 1, &buf_barrier, 0, nullptr);
}
void Gfx::CommandBuffer::MemoryBarrier(VkAccessFlags srcaccess, VkAccessFlags dstaccess, VkPipelineStageFlags srcstage, VkPipelineStageFlags dststage)
{
    VkMemoryBarrier membarrier{};
	membarrier.sType               = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	membarrier.srcAccessMask       = srcaccess;
	membarrier.dstAccessMask       = dstaccess;
// TODO : DO thing for memory pointer ---!!!!! now you use instance buffer in 2 compute shaders without proper barrier !!!!!!!!!!!!!!!
	vkCmdPipelineBarrier(cmd, srcstage, dststage, 0, 1, &membarrier, 0, nullptr, 0, nullptr);
}

void Gfx::CommandBuffer::MemoryBarrier(VkImage image, VkImageLayout oldlayout, VkImageLayout newlayout, VkImageSubresourceRange range)
{
	VkPipelineStageFlags srcstagemask  = GetPipelineStageFlags(oldlayout);
	VkPipelineStageFlags dststagemask  = GetPipelineStageFlags(newlayout);
	VkAccessFlags srcaccessmask = GetAccessFlags(oldlayout);
	VkAccessFlags dstaccessmask = GetAccessFlags(newlayout);

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

VkRenderingAttachmentInfoKHR Gfx::CommandBuffer::GetAttachmentInfo(VkImageView imageview, bool iscolor, AttachmentRules loadop)
{
    VkRenderingAttachmentInfoKHR info = {};
	VkClearValue clearvalue;
    if (iscolor) {
		clearvalue.color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
        info.pNext = nullptr;
        info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        info.imageView = imageview;
        info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        info.resolveMode = VK_RESOLVE_MODE_NONE;
        info.loadOp = ((loadop & Gfx::ATTACHMENT_RULE_LOAD) == Gfx::ATTACHMENT_RULE_LOAD) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
        info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        info.clearValue = clearvalue;
	}
    else {
		clearvalue.depthStencil = {1.0f, 0};

		info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		info.imageView = imageview;
		info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		info.resolveMode = VK_RESOLVE_MODE_NONE;
		info.loadOp = ((loadop & Gfx::ATTACHMENT_RULE_LOAD) == Gfx::ATTACHMENT_RULE_LOAD) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
		info.storeOp = VK_ATTACHMENT_STORE_OP_STORE ;
		info.clearValue = clearvalue;
	}
	return info;
}

const VkRenderingInfoKHR Gfx::CommandBuffer::GetRenderingInfo(std::vector<RenderingAttachmentInfo>& attachmentinfo, std::vector<VkRenderingAttachmentInfoKHR>& colors, VkRenderingAttachmentInfoKHR& depthinfo, Vector2<int> extents, bool multithreaded )
{
	range.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	range.baseMipLevel   = 0;
	range.levelCount     = VK_REMAINING_MIP_LEVELS;
	range.baseArrayLayer = 0;
	range.layerCount     = VK_REMAINING_ARRAY_LAYERS;

	VkImageSubresourceRange depthrange{range};
	depthrange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	VkRect2D renderarea = VkRect2D{VkOffset2D{}, { static_cast<uint32_t>(extents.x), static_cast<uint32_t>(extents.y) } };
	VkRenderingInfoKHR renderinfo {
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
		.renderArea = renderarea,
		.layerCount = 1,
	};

    if (multithreaded) {
        renderinfo.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT;
    }

    colors.reserve(attachmentinfo.size());
    for (RenderingAttachmentInfo& info : attachmentinfo) {
		if (info.IsDepth) {
			MemoryBarrier(info.Image, info.Layout, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, depthrange);
			depthinfo = GetAttachmentInfo(info.ImageView, false, info.Rules);
            depthinfo.imageView = info.ImageView;
            renderinfo.pDepthAttachment = &depthinfo;
		}
		else {
			MemoryBarrier(info.Image, info.Layout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, range);
            colors.emplace_back(GetAttachmentInfo(info.ImageView, true, info.Rules));
		}
	}
    if (!colors.empty()) {
        renderinfo.colorAttachmentCount = colors.size();
	    renderinfo.pColorAttachments = (colors.data());
    }
    return renderinfo;
}

void Gfx::CommandBuffer::CopyImage(VkImage src, Vector2<int> srcsize, VkImageLayout srcoldlayout, VkImageLayout srcnewlayout, VkImage dst, Vector2<int> dstsize, VkImageLayout dstoldlayout, VkImageLayout dstnewlayout)
{
	MemoryBarrier(src, srcoldlayout, srcnewlayout, range);
	MemoryBarrier(dst, dstoldlayout, dstnewlayout, range);

	VkImageBlit blit;
	blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	blit.srcSubresource.baseArrayLayer = 0;
	blit.srcSubresource.layerCount     = 1;
	blit.srcSubresource.mipLevel       = 0;
	blit.srcOffsets[0]                 = {0, 0, 0};
	blit.srcOffsets[1]                 = {static_cast<int>(srcsize.x), static_cast<int>(srcsize.y), 1};

	blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	blit.dstSubresource.baseArrayLayer = 0;
	blit.dstSubresource.layerCount     = 1;
	blit.dstSubresource.mipLevel       = 0;
	blit.dstOffsets[0]                 = {0, 0, 0};
	blit.dstOffsets[1]                 = {static_cast<int>(dstsize.x), static_cast<int>(dstsize.y), 1};

	vkCmdBlitImage(cmd,
		src, srcnewlayout,
		dst, dstnewlayout,
		1,
		&blit,
		VK_FILTER_NEAREST);
}
