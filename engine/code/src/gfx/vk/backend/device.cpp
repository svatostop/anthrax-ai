#include "aai/gfx/vk/backend/vk_defines.h"
#include <vulkan/vulkan_core.h>

import aai.gfx.vk.device;
import aai.utils;
import aai.utils.mem;
import std;

const std::vector<const char*> device_extenstions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME };
void vk::device::init(VkInstance vk_inst, bool validate, const std::vector<const char*>& layers)
{
    inst = vk_inst;
#ifdef AAI_LINUX
    init_linux_surface();
#else
    init_windows_surface();
#endif

    init_physical_dev();
    init_logical_dev(validate, layers);
    
    init_swapchain();
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

bool query_device_extenstions(VkPhysicalDevice device) {
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

bool is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    vk::queues::families index;
	index = find_queue_family(device, surface);
	bool extensionsupported = query_device_extenstions(device);
	bool swapchainsupport = false;
	if (extensionsupported) {
        vk::swapchain::details details = query_swapchain_support(device, surface);
		swapchainsupport = !details.formats.empty() && !details.present_modes.empty();
	}
	return index.is_done() && extensionsupported && swapchainsupport;
}
void vk::device::init_physical_dev()
{
    uint32_t devicecount = 0;
	vkEnumeratePhysicalDevices(inst, &devicecount, nullptr);

    utils::ASSERT((devicecount == 0), "failed to find GPUs with Vulkan support!");
	std::vector<VkPhysicalDevice> devices(devicecount);
	vkEnumeratePhysicalDevices(inst, &devicecount, devices.data());

	for (const auto &dev : devices) {
		if (is_device_suitable(dev, surface)) {
			physical_dev = dev;
			break;
		}
	}
	VkPhysicalDeviceProperties2 props{};
    props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
	vkGetPhysicalDeviceProperties2(physical_dev, &props);
	std::cout << "\nDevice: " << props.properties.deviceName << '\n';
	std::cout << "The GPU has a minimum buffer alignment of " << props.properties.limits.minUniformBufferOffsetAlignment << std::endl;
	std::cout << "The GPU has group size " << props.properties.limits.maxComputeWorkGroupCount[0] << std::endl;
	min_uniform_buffer_alignment = props.properties.limits.minUniformBufferOffsetAlignment;
	utils::ASSERT(physical_dev == VK_NULL_HANDLE, "failed to find a suitable GPU");
}

void vk::device::init_logical_dev(bool validate, const std::vector<const char*>& layers)
{
    vk::queues::families indices = find_queue_family(physical_dev, surface);

    std::vector<VkDeviceQueueCreateInfo> queueinfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphics.value(), indices.present.value()};
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueFamilyIndex = queueFamily;
        info.queueCount = 1;
        info.pQueuePriorities = &queuePriority;
        queueinfos.push_back(info);
    }

    VkPhysicalDeviceFeatures devicefeatures{};
	devicefeatures.samplerAnisotropy = VK_TRUE;
    devicefeatures.fragmentStoresAndAtomics = VK_TRUE;
    devicefeatures.multiDrawIndirect = VK_TRUE;
    devicefeatures.drawIndirectFirstInstance = VK_TRUE;
    
    VkPhysicalDeviceVulkan12Features features12{};
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.descriptorIndexing = true;
    
#ifdef TRACY
    features12.hostQueryReset = true;
#endif

    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynfeature{};
	dynfeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
    dynfeature.dynamicRendering = VK_TRUE;
    dynfeature.pNext = &features12;

    VkPhysicalDeviceShaderDrawParametersFeatures shaderdrawparams{};
    shaderdrawparams.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
    shaderdrawparams.pNext = &dynfeature;
    shaderdrawparams.shaderDrawParameters = VK_TRUE;

     
	VkPhysicalDeviceFeatures2 devfeatures2{};
	devfeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	devfeatures2.pNext = &shaderdrawparams;
	devfeatures2.features = devicefeatures;

	vkGetPhysicalDeviceFeatures2(physical_dev, &devfeatures2);
   
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = &devfeatures2;// &DynamicRenderingFeature,
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueinfos.size());
    createInfo.pQueueCreateInfos = queueinfos.data();
   // createInfo.pEnabledFeatures = &devicefeatures;
    int count = static_cast<uint32_t>(device_extenstions.size());
    std::vector<const char*> ext = device_extenstions; 
#ifdef TRACY
        count++;
        ext.push_back("VK_EXT_calibrated_timestamps");
        count++;
        ext.push_back("VK_EXT_host_query_reset");
#endif
    createInfo.enabledExtensionCount = static_cast<uint32_t>(count);
    createInfo.ppEnabledExtensionNames = ext.data();
    
    if (validate) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        createInfo.ppEnabledLayerNames = layers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    utils::VK_ASSERT(vkCreateDevice(physical_dev, &createInfo, nullptr, &dev), "failed to create logical device!");

	vkGetDeviceQueue(dev, indices.graphics.value(), 0, &queue.graphics);
	vkGetDeviceQueue(dev, indices.present.value(), 0, &queue.present);
}
