module;
#include "aai/gfx/vk/backend/vk_defines.h"

export module aai.gfx.vk.device.swapchain;
import std;
export {
    namespace vk {
        namespace swapchain {
            struct details {
                VkSurfaceCapabilitiesKHR		capabilities;
                std::vector<VkSurfaceFormatKHR>	formats;
                std::vector<VkPresentModeKHR>	present_modes;
            };
            struct handlers {
                VkSwapchainKHR				swapchain;
	            VkFormat 					format;
	            std::vector<VkImage>		images;
	            std::vector<VkImageView> 	image_views;
	            VkExtent2D 			    	extent;
            };
            vk::swapchain::details query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface)
            {
                vk::swapchain::details  detail;
                vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &detail.capabilities);
                uint32_t formatcount;
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatcount, nullptr);
                if (formatcount != 0) {
                    detail.formats.resize(formatcount);
                    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatcount, detail.formats.data());
                }
                uint32_t presentmodecount;
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentmodecount, nullptr);
                if (presentmodecount != 0) {
                    detail.present_modes.resize(presentmodecount);
                    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentmodecount, detail.present_modes.data());
                }
                return detail;
            }
            VkSurfaceFormatKHR get_format(const std::vector<VkSurfaceFormatKHR>& availableFormats)
            {
                for (const auto& availableFormat : availableFormats) {
                    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
                    && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                        return availableFormat;
                }
                return availableFormats[0];
            }
            VkPresentModeKHR get_present_mode(const std::vector<VkPresentModeKHR>& availablepresentmodes)
            {
                for (const auto& availablePresentMode : availablepresentmodes) {
                    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                        return availablePresentMode;
                }
                return VK_PRESENT_MODE_FIFO_KHR;
            }

            VkExtent2D get_extents(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D resolution)
            {
                if (capabilities.currentExtent.width != UINT32_MAX)
                    return capabilities.currentExtent;
                else {
                    VkExtent2D extent = resolution; 
                    extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
                    extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
                    return extent;
                }
            }

         }
    }
};
