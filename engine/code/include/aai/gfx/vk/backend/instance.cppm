module;
#include "aai/gfx/vk/backend/vk_defines.h"

export module aai.gfx.vk.instance;
import std;
export {
    namespace vk {
        VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
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
        std::vector<const char*> instances_ext =
{VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_xcb_surface", VK_EXT_DEBUG_UTILS_EXTENSION_NAME, "VK_KHR_get_physical_device_properties2"};

        class instance {
            public:
                void init(bool validate);

                VkInstance get_instance() { return vk_instance; }
                const std::vector<const char*>& get_layers() { return layers; }
            private:
                bool enum_validation_layer_support();
                bool enum_instance_ext_support();

                VkInstance vk_instance;

                VkDebugUtilsMessengerEXT debug_messenger;
                
                std::vector<const char*> layers = { "VK_LAYER_KHRONOS_validation" };
        };
    }
};

