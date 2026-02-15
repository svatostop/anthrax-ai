#include <iostream>
#include <cstring>
#include <vector>
#include <vulkan/vulkan.h>
#ifdef AAI_WINDOWS
#include <windows.h>
#include <backends/imgui_impl_win32.h>
#include <vulkan/vulkan_win32.h>
#endif
#ifdef AAI_LINUX
#include <xcb/xcb.h>
#include <vulkan/vulkan_xcb.h>

const std::vector<const char*> instances_ext =
{VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_xcb_surface", VK_EXT_DEBUG_UTILS_EXTENSION_NAME, "VK_KHR_get_physical_device_properties2"};
#else
const std::vector<const char*> INSTANCE_EXT =
{VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_win32_surface", VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
#endif

#include <vulkan/vk_enum_string_helper.h>
const std::vector<const char*> layers = { "VK_LAYER_KHRONOS_validation" };
const std::vector<const char*> device_ext = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME };

#define ASSERT(x, s)	                                        \
do                                                              \
{                                                               \
	bool err = x;												\
	std::string str = s;	                                    \
	if (err)                                                   	\
	{                                                           \
		std::string errstr = "Error: " + str;					\
		errstr += "\n\n";										\
		throw std::runtime_error(errstr);						\
	}                                                           \
} while (0)
#define VK_ASSERT(x, s)                                         \
do                                                              \
{                                                               \
	VkResult err = x;                                           \
	std::string str = s;	                                    \
	if (err)                                                    \
	{        													\
		std::string vulkan = string_VkResult(err);              \
		std::string errstr = "Vulkan: Error: " + vulkan;		\
		errstr += "\n\n" + str;									\
		throw std::runtime_error(errstr);						\
	}                                                           \
} while (0)

