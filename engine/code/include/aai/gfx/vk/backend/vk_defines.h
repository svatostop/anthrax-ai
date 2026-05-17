#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.h>
#include <stdio.h>
#ifdef AAI_WINDOWS
#include <windows.h>
#include <backends/imgui_impl_win32.h>
#include <vulkan/vulkan_win32.h>
#endif
#ifdef AAI_LINUX
// #include <vulkan/vulkan_xlib.h>
#else
#endif

#define MAX_FRAMES 3
#define MAX_BINDING 3
