#pragma once

#include "anthraxAI/utils/defines.h"
#include "anthraxAI/gfx/vkdefines.h"
#include <string>
#include <vulkan/vulkan_core.h>

namespace Gfx
{
    class VkDebug
    {
        public:
            VkDebug();

            void Setup(VkInstance instance, bool validationon);
            VkDebugUtilsMessengerCreateInfoEXT* GetInfo() { return &MessengerCreateInfo; }
            void Destroy(VkInstance instance, const VkAllocationCallbacks* pAllocator);
            void SetName(VkDebugUtilsObjectNameInfoEXT info);
            void SetRTName(const std::string& name, VkImage image);
            void SetRenderName(VkCommandBuffer cmd, VkDebugUtilsLabelEXT* info);
            void EndRenderName(VkCommandBuffer cmd);
            
            void SetDebugName(const std::string& name, uint64_t handle, VkObjectType type);
        private:
            VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) ;
            PFN_vkSetDebugUtilsObjectNameEXT SetDebugUtilsObjectNameEXT;
            PFN_vkCmdBeginDebugUtilsLabelEXT SetBeginDebugUtilsLabelEXT;
            PFN_vkCmdEndDebugUtilsLabelEXT SetEndDebugUtilsLabelEXT;
                        
            VkDebugUtilsMessengerEXT Messenger;
            VkDebugUtilsMessengerCreateInfoEXT MessengerCreateInfo;
    };
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
}
