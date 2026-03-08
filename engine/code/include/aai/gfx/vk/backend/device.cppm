module;
#include "aai/gfx/vk/backend/vk_defines.h"

export module aai.gfx.vk.device;
import std;
export {
   namespace vk {
        namespace swapchain {
            struct details {
                VkSurfaceCapabilitiesKHR		capabilities;
                std::vector<VkSurfaceFormatKHR>	formats;
                std::vector<VkPresentModeKHR>	present_modes;
            };
        };
        namespace queues {
            struct handles {
                VkQueue graphics;
                VkQueue present;
            };
            enum type {
                GRAPHICS = 0,
                PRESENT
            };
            struct families {
                std::optional<uint32_t> graphics;
                std::optional<uint32_t> present;

                bool is_done() {
                    return graphics.has_value()
                    && present.has_value();
                }
            };
        }
       class device {
            public:
                void init(VkInstance vk_inst, bool validate, const std::vector<const char*>& layers);
            private:
#ifdef AAI_LINUX
                void init_linux_surface() {}
#else
                void init_windows_surface() {}
#endif  
                void init_physical_dev();
                void init_logical_dev(bool validate, const std::vector<const char*>& layers);
                void init_swapchain() {}

                VkInstance inst;
                VkSurfaceKHR surface;
                VkPhysicalDevice physical_dev;
                VkDevice dev;
                queues::handles queue;

    	        size_t min_uniform_buffer_alignment;
       };
   }
};

