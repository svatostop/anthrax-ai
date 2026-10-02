module;
#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.h>
#include "backends/imgui_impl_glfw.h"

export module aai.editor;
import aai.editor.imgui;

export {
    namespace aai {
        class editor {
            public:
                void shutdown(VkDevice device) {
                    if (is_init)
                        imgui.shutdown(device);
                }
                void init(GLFWwindow* window, VkInstance instance, VkPhysicalDevice pshysical_device,VkDevice device, VkQueue queue, std::uint32_t queue_family) {
                    imgui.init(window, instance, pshysical_device, device, queue, queue_family);
                    is_init = true;
                }

                void run();
                void backend_render_data(VkCommandBuffer cmd) { imgui.backend_render_data(cmd); }
            private:
                bool is_init = false;
                imgui_backend imgui;
        };
    }
};
