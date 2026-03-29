#include <cstring>
#include "aai/gfx/vk/backend/vk_defines.h" 
#include <vulkan/vulkan_core.h>
import aai.gfx.vk.instance;
import aai.utils;
import aai.utils.mem;
import std;

VkResult setup_debug_layer(VkInstance inst, VkDebugUtilsMessengerEXT* messanger, VkDebugUtilsMessengerCreateInfoEXT* info)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(inst, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(inst, info, nullptr, messanger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void destroy_debug_layer(VkInstance inst, VkDebugUtilsMessengerEXT messanger) {

    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(inst, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
       func(inst, messanger, nullptr);
    } 
}

void vk::instance::init(bool validate)
{
    VkApplicationInfo appinfo{};
    appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appinfo.pApplicationName = "aai";
    appinfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appinfo.pEngineName = "anthaxAI";
    appinfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appinfo.apiVersion = VK_API_VERSION_1_3;
 
    VkInstanceCreateInfo createinfo{};
	createinfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createinfo.pApplicationInfo = &appinfo;

    if (!instances_ext.empty()) {
        utils::ASSERT(!enum_instance_ext_support(), "Not supported required instance extensions!");
        createinfo.ppEnabledExtensionNames = instances_ext.data();
        createinfo.enabledExtensionCount = static_cast<uint32_t>(instances_ext.size());
    }
    
	utils::ASSERT((validate && !enum_validation_layer_support()), "Not supported validation layers!");
    
    VkDebugUtilsMessengerCreateInfoEXT info;
    if (validate) {
        info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        info.pfnUserCallback = vk_debug_callback;
        info.pUserData = nullptr;

        createinfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        createinfo.ppEnabledLayerNames = layers.data();
        createinfo.pNext = &info;
    }
    else {
        createinfo.enabledLayerCount = 0;
        createinfo.pNext = nullptr;
    }
	
    utils::VK_ASSERT(vkCreateInstance(&createinfo, nullptr, &vk_instance), "vkCreateInstance failed");
    utils::mem::get()->push(utils::mem::event::DELETE, utils::mem::type::VK, [=,this]() { vkDestroyInstance(vk_instance, nullptr); });

    if (validate) {
        utils::VK_ASSERT(setup_debug_layer(vk_instance, &debug_messenger, &info), "vk debug setup failed");
        utils::mem::get()->push(utils::mem::event::DELETE, utils::mem::type::VK, [=,this]() { destroy_debug_layer(vk_instance, debug_messenger); });
    }

    SetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(vk_instance, "vkSetDebugUtilsObjectNameEXT");
    SetBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(vk_instance, "vkCmdBeginDebugUtilsLabelEXT" );
    SetEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(vk_instance, "vkCmdEndDebugUtilsLabelEXT" );
}

bool vk::instance::enum_validation_layer_support()
{
   uint32_t layercount;
	vkEnumerateInstanceLayerProperties(&layercount, nullptr);

	std::vector<VkLayerProperties> availablelayers(layercount);
	vkEnumerateInstanceLayerProperties(&layercount, availablelayers.data());

	for (const char* layername : layers) {
	    bool found = false;

	    for (const auto& layerproperties : availablelayers) {
	        if (strcmp(layername, layerproperties.layerName) == 0) {
	            found = true;
	            break;
	        }
	    }
	    if (!found) {
	        return false;
	    }
	}
	return true;
}

bool vk::instance::enum_instance_ext_support()
{
    unsigned int instextcount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &instextcount, nullptr);

	std::vector<VkExtensionProperties> availableextensions(instextcount);
	vkEnumerateInstanceExtensionProperties(nullptr, &instextcount, availableextensions.data());

	for (const char *requiredextname : instances_ext) {
		bool found = false;
		for (const VkExtensionProperties &extproperties : availableextensions) {
			if (strcmp(requiredextname, extproperties.extensionName) == 0) {
				found = true;
				break;
			}
		}
		if (!found) {
			return false;
		}
	}
	return true;
}
