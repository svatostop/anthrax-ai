module;
#include "aai/gfx/vk/backend/vk_defines.h"
#include "aai/io/win_defines.h"

export module aai.gfx.vk.device;
import aai.gfx.vk.device.helper;
import aai.gfx.vk.device.swapchain;
import std;
export {
   namespace vk {
       class device {
            public:
                struct handlers {
                    VkDevice dev;
                    VkPhysicalDevice physical_dev;
                };

                void init(bool validate, const std::vector<const char*>& layers);
#ifdef AAI_LINUX
                void init_linux_surface(VkInstance vk_inst, Display* di, Window w);
#else
                void init_windows_surface() {}
#endif
                handlers get_devices() const { return devices; }
                VkDevice get_device() const { return devices.dev; }
                const uint32_t get_graphics_index() const ;
            private:
                void init_physical_dev();
                void init_logical_dev(bool validate, const std::vector<const char*>& layers);
                void init_swapchain();

                VkInstance inst;
                VkSurfaceKHR surface;
                handlers devices;
                swapchain::handlers sw; 
                queues::handlers queue;

    	        size_t min_uniform_buffer_alignment;
                const std::vector<const char*> device_extenstions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME };
       };
   }
};

