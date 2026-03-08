#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.h>
#ifdef AAI_WINDOWS
#include <windows.h>
#include <backends/imgui_impl_win32.h>
#include <vulkan/vulkan_win32.h>
#endif
#ifdef AAI_LINUX
#include <xcb/xcb.h>
#include <vulkan/vulkan_xcb.h>

#else
#endif

