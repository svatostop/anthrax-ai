#pragma once

#include <vector>
#include <cstring>

#include <vulkan/vulkan.h>
#ifdef AAI_WINDOWS
#include <windows.h>
#include <backends/imgui_impl_win32.h>
#include <vulkan/vulkan_win32.h>
#endif
#include <vulkan/vk_enum_string_helper.h>
#ifdef AAI_LINUX
#include <xcb/xcb.h>
#include <vulkan/vulkan_xcb.h>
const std::vector<const char*> INSTANCE_EXT =
{VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_xcb_surface", VK_EXT_DEBUG_UTILS_EXTENSION_NAME, "VK_KHR_get_physical_device_properties2"};
#else
const std::vector<const char*> INSTANCE_EXT =
{VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_win32_surface", VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
#endif

#define MAX_FRAMES 3
#define COMPUTE_SKINNING

const std::vector<const char*> VALIDATION_LAYER = { "VK_LAYER_KHRONOS_validation" };
const std::vector<const char*> DEVICE_EXT = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME };


