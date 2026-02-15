import aai.gfx.vk.instance;
#include "aai/gfx/vk/backend/vk_defines.h" 

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
        ASSERT(!enum_instance_ext_support(), "Not supported required instance extensions!");
        createinfo.ppEnabledExtensionNames = instances_ext.data();
        createinfo.enabledExtensionCount = static_cast<uint32_t>(instances_ext.size());
    }
    
	ASSERT((validate && !enum_validation_layer_support()), "Not supported validation layers!");
    if (validate) {
        createinfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        createinfo.ppEnabledLayerNames = layers.data();
        // createinfo.pNext = Debug.GetInfo();
    }
    else {
        createinfo.enabledLayerCount = 0;
        createinfo.pNext = nullptr;
    }
	
    VK_ASSERT(vkCreateInstance(&createinfo, nullptr, &vk_instance), "vkCreateInstance failed");
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
