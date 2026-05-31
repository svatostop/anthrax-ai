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
                void init_linux_surface(VkInstance vk_inst,GLFWwindow* glwf_w,  Display* di, Window w);
#else
                void init_windows_surface() {}
#endif
                handlers get_devices() const { return devices; }
                VkDevice get_device() const { return devices.dev; }
                const uint32_t get_graphics_index() const ;
                
                VkImage get_swapchain_image(uint32_t image) { return sw.images[image]; }
                VkExtent2D get_swapchain_size() { return sw.extent; } 
                uint32_t get_swapchains_amount() { return sw.images.size(); } 
                VkSwapchainKHR get_swapchain() { return sw.swapchain; }
                VkQueue get_queue(vk::queues::type type) { return queue.q[type]; }

                void on_resize();
            private:
                void init_physical_dev();
                void init_logical_dev(bool validate, const std::vector<const char*>& layers);
                void init_swapchain();

                VkInstance inst;
                VkSurfaceKHR surface;
                handlers devices;
                GLFWwindow* glfw_win = nullptr;
                int width = 0;
                int height = 0;
                swapchain::handlers sw; 
                queues::handlers queue;

    	        size_t min_uniform_buffer_alignment;
                const std::vector<const char*> device_extenstions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME };
       };
   }
};

