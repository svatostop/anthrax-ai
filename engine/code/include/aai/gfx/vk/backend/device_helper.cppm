module;
#include "aai/gfx/vk/backend/vk_defines.h"

export module aai.gfx.vk.device.helper;
import aai.gfx.vk.device.swapchain;
import std;
export {
   namespace vk {
        namespace queues {
            struct handles {
                VkQueue graphics;
                VkQueue present;
            };
            enum type {
                GRAPHICS = 0,
                PRESENT,
                SIZE
            };
            struct families {
                std::optional<uint32_t> graphics;
                std::optional<uint32_t> present;

                bool is_done() {
                    return graphics.has_value()
                    && present.has_value();
                }
            };
        }
vk::queues::families find_queue_family(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    vk::queues::families index;
	uint32_t queuefamilycount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queuefamilycount, nullptr);

	std::vector<VkQueueFamilyProperties> queuefamilies(queuefamilycount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queuefamilycount, queuefamilies.data());
	int ind = 0;
	for (const auto& queuefam : queuefamilies) {
		if (index.is_done()) {
			break ;
		}
		if (queuefam.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			index.graphics = ind;
		}
		VkBool32 presentsupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, ind, surface, &presentsupport);
		if (presentsupport) {
			index.present = ind;
		}
		ind++;
	}
	return index;
}

bool query_device_extenstions(VkPhysicalDevice device, const std::vector<const char*>& device_extenstions) {
        uint32_t extensioncount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensioncount, nullptr);
        std::vector<VkExtensionProperties> availableextensions(extensioncount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensioncount, availableextensions.data());

        std::set<std::string> requiredextensions(device_extenstions.begin(), device_extenstions.end());
        for (const auto& extension : availableextensions) {
            requiredextensions.erase(extension.extensionName);
        }
        return requiredextensions.empty();
}


bool is_device_suitable(const std::vector<const char*>& device_extenstions, VkPhysicalDevice device, VkSurfaceKHR surface)
{
    vk::queues::families index;
	index = find_queue_family(device, surface);
	bool extensionsupported = query_device_extenstions(device, device_extenstions);
	bool swapchainsupport = false;
	if (extensionsupported) {
        vk::swapchain::details details = vk::swapchain::query_swapchain_support(device, surface);
		swapchainsupport = !details.formats.empty() && !details.present_modes.empty();
	}
	return index.is_done() && extensionsupported && swapchainsupport;
}

    }
};
