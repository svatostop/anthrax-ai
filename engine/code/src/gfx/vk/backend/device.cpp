#include "aai/gfx/vk/backend/vk_defines.h"
#include "aai/io/win_defines.h"

import aai.gfx.vk.device;
import aai.gfx.vk.device.helper;
import aai.gfx.vk.device.swapchain;
import aai.utils;
import aai.utils.mem;
import std;

void vk::device::init(bool validate, const std::vector<const char*>& layers)
{
    init_physical_dev();
    init_logical_dev(validate, layers);
    
    init_swapchain();
}

void vk::device::init_linux_surface(VkInstance vk_inst, Display* di, Window w)
{
    inst = vk_inst;
    VkXlibSurfaceCreateInfoKHR surface_info{};
    surface_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    surface_info.pNext = nullptr;
    surface_info.dpy = di;
    surface_info.window = w;

    utils::VK_ASSERT(vkCreateXlibSurfaceKHR(inst, &surface_info, nullptr, &surface), "vkCreateXlibSurfaceKHR failed!");
    utils::mem::get()->push(utils::mem::event::DELETE, utils::mem::type::VK, [=,this]() { vkDestroySurfaceKHR(inst, surface, nullptr); });
}

const uint32_t vk::device::get_graphics_index() const 
{ 
    vk::queues::families indices = find_queue_family(devices.physical_dev, surface); 
    return indices.graphics.value();
} 
void vk::device::init_swapchain()
{
    vk::queues::families indices = find_queue_family(devices.physical_dev, surface);
    uint32_t image_count = 0;
    vk::swapchain::details swapchainsupport = vk::swapchain::query_swapchain_support(devices.physical_dev, surface);
    VkSurfaceFormatKHR surfaceFormat = vk::swapchain::get_format(swapchainsupport.formats);
    VkPresentModeKHR presentMode = vk::swapchain::get_present_mode(swapchainsupport.present_modes);
    VkExtent2D extent =  vk::swapchain::get_extents(swapchainsupport.capabilities, {800, 800});

    image_count = swapchainsupport.capabilities.minImageCount + 1;
    if (swapchainsupport.capabilities.maxImageCount > 0 && image_count > swapchainsupport.capabilities.maxImageCount)
        image_count = swapchainsupport.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = image_count;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    uint32_t queueFamilyIndices[] = {indices.graphics.value(), indices.present.value()};
    if (indices.graphics != indices.present) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = queues::type::SIZE;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }
    createInfo.preTransform = swapchainsupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    utils::VK_ASSERT(vkCreateSwapchainKHR(devices.dev, &createInfo, nullptr, &sw.swapchain), "failed to create swap chain!");
    utils::mem::get()->push(utils::mem::event::DELETE, utils::mem::type::VK, [=,this]() { vkDestroySwapchainKHR(devices.dev, sw.swapchain, nullptr);; });
	
    vkGetSwapchainImagesKHR(devices.dev, sw.swapchain, &image_count, nullptr);
	sw.images.resize(image_count);
	vkGetSwapchainImagesKHR(devices.dev, sw.swapchain, &image_count, sw.images.data());
	sw.format = surfaceFormat.format;
	sw.extent = extent;

    sw.image_views.resize(sw.images.size());
	for (size_t i = 0; i < sw.images.size(); i++) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = sw.images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = sw.format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
        utils::VK_ASSERT(vkCreateImageView(devices.dev, &createInfo, nullptr, &sw.image_views[i]), "failed to create image view!");
        utils::mem::get()->push(utils::mem::event::DELETE, utils::mem::type::VK, [=,this]() { vkDestroyImageView(devices.dev, sw.image_views[i], nullptr); });
	}
}

void vk::device::init_physical_dev()
{
    uint32_t devicecount = 0;
	vkEnumeratePhysicalDevices(inst, &devicecount, nullptr);

    utils::ASSERT((devicecount == 0), "failed to find GPUs with Vulkan support!");
	std::vector<VkPhysicalDevice> ddevices(devicecount);
	vkEnumeratePhysicalDevices(inst, &devicecount, ddevices.data());

	for (const auto &dev : ddevices) {
		if (is_device_suitable(device_extenstions, dev, surface)) {
			devices.physical_dev = dev;
			break;
		}
	}
	VkPhysicalDeviceProperties2 props{};
    props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
	vkGetPhysicalDeviceProperties2(devices.physical_dev, &props);
	std::cout << "\nDevice: " << props.properties.deviceName << '\n';
	std::cout << "The GPU has a minimum buffer alignment of " << props.properties.limits.minUniformBufferOffsetAlignment << std::endl;
	std::cout << "The GPU has group size " << props.properties.limits.maxComputeWorkGroupCount[0] << std::endl;
	min_uniform_buffer_alignment = props.properties.limits.minUniformBufferOffsetAlignment;
	utils::ASSERT(devices.physical_dev == VK_NULL_HANDLE, "failed to find a suitable GPU");
}

void vk::device::init_logical_dev(bool validate, const std::vector<const char*>& layers)
{
    vk::queues::families indices = find_queue_family(devices.physical_dev, surface);

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
    features12.bufferDeviceAddress = true;
    features12.timelineSemaphore = true;
#ifdef TRACY
    features12.hostQueryReset = true;
#endif

    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynfeature{};
	dynfeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
    dynfeature.dynamicRendering = VK_TRUE;
    dynfeature.pNext = &features12;
    
    // VkPhysicalDeviceTimelineSemaphoreFeatures timelineFeatures{};
    // timelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
    // timelineFeatures.timelineSemaphore = VK_TRUE;
    // timelineFeatures.pNext = &dynfeature;
    // VkPhysicalDeviceBufferDeviceAddressFeatures buffer_device_addrs_feature{};
    // buffer_device_addrs_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
    // buffer_device_addrs_feature.bufferDeviceAddress = true;
    // buffer_device_addrs_feature.pNext = &dynfeature;
    //
    VkPhysicalDeviceShaderDrawParametersFeatures shaderdrawparams{};
    shaderdrawparams.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
    shaderdrawparams.pNext = &dynfeature;
    shaderdrawparams.shaderDrawParameters = VK_TRUE;
     
	VkPhysicalDeviceFeatures2 devfeatures2{};
	devfeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	devfeatures2.pNext = &shaderdrawparams;
	devfeatures2.features = devicefeatures;

	vkGetPhysicalDeviceFeatures2(devices.physical_dev, &devfeatures2);
   
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

    utils::VK_ASSERT(vkCreateDevice(devices.physical_dev, &createInfo, nullptr, &devices.dev), "failed to create logical device!");
    utils::mem::get()->push(utils::mem::event::DELETE, utils::mem::type::VK, [=,this]() { vkDestroyDevice(devices.dev, nullptr); });

	vkGetDeviceQueue(devices.dev, indices.graphics.value(), 0, &queue.q[queues::type::GRAPHICS]);
	vkGetDeviceQueue(devices.dev, indices.present.value(), 0, &queue.q[queues::type::PRESENT]);
}
