#include "anthraxAI/gfx/vkdebug.h"
#include "anthraxAI/gfx/vkdevice.h"
#include "anthraxAI/utils/tracy.h"
#include <vulkan/vulkan_core.h>

static VKAPI_ATTR VkBool32 VKAPI_CALL Gfx::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    if (pCallbackData->messageIdNumber != 3357201678) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << "\n----------------------------------\n" << std::endl;
    }
    return VK_FALSE;
}

Gfx::VkDebug::VkDebug()
{
	MessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	MessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	MessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	MessengerCreateInfo.pfnUserCallback = DebugCallback;
	MessengerCreateInfo.pUserData = nullptr;
}

void Gfx::VkDebug::Setup(VkInstance instance, bool validationon)
{
    if (!validationon) {
        return;
    }
	VK_ASSERT(CreateDebugUtilsMessengerEXT(instance, &MessengerCreateInfo, nullptr, &Messenger), "failed to set up debug messenger!");
}

void Gfx::VkDebug::SetDebugName(const std::string& name, uint64_t handle, VkObjectType type)
{
    VkDebugUtilsObjectNameInfoEXT info;
	info.pNext = nullptr;
	info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	info.objectHandle = handle;
	info.objectType = type;
	info.pObjectName = name.c_str();
    SetName(info);
}

void Gfx::VkDebug::SetRTName(const std::string& name, VkImage image)
{
    VkDebugUtilsObjectNameInfoEXT info;
	info.pNext = nullptr;
	info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	info.objectHandle = reinterpret_cast<uint64_t>(image);
	info.objectType = VK_OBJECT_TYPE_IMAGE;
	info.pObjectName = name.c_str();
    SetName(info);
}

void Gfx::VkDebug::SetName(VkDebugUtilsObjectNameInfoEXT info)
{
    SetDebugUtilsObjectNameEXT(Gfx::Device::GetInstance()->GetDevice(), &info);
}

void Gfx::VkDebug::SetRenderName(VkCommandBuffer cmd, VkDebugUtilsLabelEXT* info)
{
    SetBeginDebugUtilsLabelEXT(cmd, info);
}

void Gfx::VkDebug::EndRenderName(VkCommandBuffer cmd)
{
    SetEndDebugUtilsLabelEXT(cmd);
}

void Gfx::VkDebug::Destroy(VkInstance instance, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, Messenger, pAllocator);
    }
}

VkResult Gfx::VkDebug::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    SetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
    SetBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT" );
    SetEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT" );

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}
