#include "anthraxAI/gfx/vkdevice.h"
#include "anthraxAI/gfx/vkdefines.h"
#include "anthraxAI/gfx/vkdevicehelper.h"
#include "anthraxAI/utils/tracy.h"
#include <cstdio>
#include <vulkan/vulkan_core.h>

void Gfx::Device::Init()
{
#ifdef AAI_LINUX
    CreateLinuxSurface();
#else
    CreateWindowsSurface();
#endif

    CreatePhysicalDevice();
    CreateDevice();

    CreateSwapchain();
    CreateSwapchainImageViews();
}

void Gfx::Device::CleanUpSwapchain()
{
	vkDestroySwapchainKHR(LogicalDevice, Swapchain.Swapchain, nullptr);

	for (size_t i = 0; i < Swapchain.Images.size(); i++) {
		vkDestroyImageView(LogicalDevice, Swapchain.ImageViews[i], nullptr);
	}
}

void Gfx::Device::CreatePhysicalDevice()
{
	uint32_t devicecount = 0;
	vkEnumeratePhysicalDevices(Gfx::Vulkan::GetInstance()->GetVkInstance(), &devicecount, nullptr);

	ASSERT((devicecount == 0), "failed to find GPUs with Vulkan support!");
	std::vector<VkPhysicalDevice> devices(devicecount);
	vkEnumeratePhysicalDevices(Gfx::Vulkan::GetInstance()->GetVkInstance(), &devicecount, devices.data());

	for (const auto &dev : devices) {
		if (IsDeviceSuitable(dev)) {
			PhysicalDevice = dev;
			break;
		}
	}
	VkPhysicalDeviceProperties2 props{};
    props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
	vkGetPhysicalDeviceProperties2(PhysicalDevice, &props);
	std::cout << "\nDevice: " << props.properties.deviceName << '\n';
	std::cout << "The GPU has a minimum buffer alignment of " << props.properties.limits.minUniformBufferOffsetAlignment << std::endl;
	std::cout << "The GPU has group size " << props.properties.limits.maxComputeWorkGroupCount[0] << std::endl;
	MinUniformBufferOffsetAlignment = props.properties.limits.minUniformBufferOffsetAlignment;
	ASSERT(PhysicalDevice == VK_NULL_HANDLE, "failed to find a suitable GPU");
}

void Gfx::Device::CreateDevice()
{
    QueueFamilyIndex indices = FindQueueFamilies(PhysicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueinfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.Graphics.value(), indices.Present.value()};
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

	vkGetPhysicalDeviceFeatures2(PhysicalDevice, &devfeatures2);
   
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = &devfeatures2;// &DynamicRenderingFeature,
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueinfos.size());
    createInfo.pQueueCreateInfos = queueinfos.data();
   // createInfo.pEnabledFeatures = &devicefeatures;
    int count = static_cast<uint32_t>(DEVICE_EXT.size());
    std::vector<const char*> ext = DEVICE_EXT; 
#ifdef TRACY
        printf("ssdsdsdsdsdsdsd\n");
        count++;
        ext.push_back("VK_EXT_calibrated_timestamps");
        count++;
        ext.push_back("VK_EXT_host_query_reset");
#endif
    createInfo.enabledExtensionCount = static_cast<uint32_t>(count);
    createInfo.ppEnabledExtensionNames = ext.data();

    // if (Gfx::Vulkan::GetInstance()->IsValidationLayersOn()) {
    //     createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYER.size());
    //     createInfo.ppEnabledLayerNames = VALIDATION_LAYER.data();
    // } else {
        createInfo.enabledLayerCount = 0;
    // }

	VK_ASSERT(vkCreateDevice(PhysicalDevice, &createInfo, nullptr, &LogicalDevice), "failed to create logical device!");

	vkGetDeviceQueue(LogicalDevice, indices.Graphics.value(), 0, &Queue.Graphics);
	vkGetDeviceQueue(LogicalDevice, indices.Present.value(), 0, &Queue.Present);
}

void Gfx::Device::CreateSwapchain()
{
	SwapChainSupportDetails swapchainsupport = QuerySwapchainSupport(PhysicalDevice, Surface);
    VkSurfaceFormatKHR surfaceFormat = ChooseSwapchainSurfaceFormat(swapchainsupport.Formats);
    VkPresentModeKHR presentMode = ChooseSwapchainPresentMode(swapchainsupport.Presentmodes);
    VkExtent2D extent = ChooseSwapchainExtent(swapchainsupport.Capabilities, Core::WindowManager::GetInstance()->GetScreenResolution());

    uint32_t imagecount = swapchainsupport.Capabilities.minImageCount + 1;
    if (swapchainsupport.Capabilities.maxImageCount > 0 && imagecount > swapchainsupport.Capabilities.maxImageCount)
    	imagecount = swapchainsupport.Capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = Surface;
	createInfo.minImageCount = imagecount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	QueueFamilyIndex indices = FindQueueFamilies(PhysicalDevice);
	uint32_t queueFamilyIndices[] = {indices.Graphics.value(), indices.Present.value()};
	if (indices.Graphics != indices.Present) {
	    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
	    createInfo.queueFamilyIndexCount = 2;
	    createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
	    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	    createInfo.queueFamilyIndexCount = 0;
	    createInfo.pQueueFamilyIndices = nullptr;
	}
	createInfo.preTransform = swapchainsupport.Capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VK_ASSERT(vkCreateSwapchainKHR(LogicalDevice, &createInfo, nullptr, &Swapchain.Swapchain), "failed to create swap chain!");

	vkGetSwapchainImagesKHR(LogicalDevice, Swapchain.Swapchain, &imagecount, nullptr);
	Swapchain.Images.resize(imagecount);
	vkGetSwapchainImagesKHR(LogicalDevice, Swapchain.Swapchain, &imagecount, Swapchain.Images.data());
	Swapchain.Format = surfaceFormat.format;
	Swapchain.Extent = extent;
}

void Gfx::Device::CreateSwapchainImageViews() {
	Swapchain.ImageViews.resize(Swapchain.Images.size());
	for (size_t i = 0; i < Swapchain.Images.size(); i++) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = Swapchain.Images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = Swapchain.Format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
		VK_ASSERT(vkCreateImageView(LogicalDevice, &createInfo, nullptr, &Swapchain.ImageViews[i]), "failed to create image view!");
	}
}

#ifdef AAI_LINUX
void Gfx::Device::CreateLinuxSurface()
{
    VkXcbSurfaceCreateInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	info.pNext = NULL;
	info.flags = 0;
	info.connection = Core::WindowManager::GetInstance()->GetConnection();
	info.window = *(Core::WindowManager::GetInstance()->GetWindow());

	VK_ASSERT(vkCreateXcbSurfaceKHR(Gfx::Vulkan::GetInstance()->GetVkInstance(), &info, NULL, &Surface), "failed to create window surface!");
}
#else
void Gfx::Device::CreateWindowsSurface()
{
    VkWin32SurfaceCreateInfoKHR info{};
	info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	info.hwnd = Core::WindowManager::GetInstance()->GetWinWindow();
	info.hinstance = Core::WindowManager::GetInstance()->GetWinInstance();

	VK_ASSERT(vkCreateWin32SurfaceKHR(Gfx::Vulkan::GetInstance()->GetVkInstance(), &info, nullptr, &Surface), "failed to create window surface!");
}
#endif

Gfx::QueueFamilyIndex Gfx::Device::FindQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndex index;
	uint32_t queuefamilycount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queuefamilycount, nullptr);

	std::vector<VkQueueFamilyProperties> queuefamilies(queuefamilycount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queuefamilycount, queuefamilies.data());
	int ind = 0;
	for (const auto& queuefam : queuefamilies) {
		if (index.IsComplete()) {
			break ;
		}
		if (queuefam.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			index.Graphics = ind;
		}
		VkBool32 presentsupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, ind, Surface, &presentsupport);
		if (presentsupport) {
			index.Present = ind;
		}
		ind++;
	}
	return index;
}

void Gfx::Device::RecreateSwapchain()
{
    if (!Gfx::Renderer::GetInstance()->IsOnResize()) {
        return;
    }
    Gfx::Renderer::GetInstance()->SetOnResize(false);
    
    vkDeviceWaitIdle(LogicalDevice);

    CleanUpSwapchain();
    CreateSwapchain();
    CreateSwapchainImageViews();
    printf("swapchain recreated\n");
}

VkQueue Gfx::Device::GetQueue(QueuesEnum q)
{
	if (q == GRAPHICS_QUEUE) {
		return Queue.Graphics;
	}
	return Queue.Present;
}

bool Gfx::Device::IsDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndex index;
	index = FindQueueFamilies(device);
	bool extensionsupported = DeviceExtSupport(device);
	bool swapchainsupport = false;
	if (extensionsupported) {
		SwapChainSupportDetails details = QuerySwapchainSupport(device, Surface);
		swapchainsupport = !details.Formats.empty() && !details.Presentmodes.empty();
	}
	return index.IsComplete() && extensionsupported && swapchainsupport;
}
