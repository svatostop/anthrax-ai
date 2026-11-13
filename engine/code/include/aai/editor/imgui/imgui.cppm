module;
#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.h>
#include "backends/imgui_impl_glfw.h"

export module aai.editor.imgui;
export import std;
export {
    class imgui_backend {
        public:
            void shutdown(VkDevice device) ;
            void init(GLFWwindow* window, VkInstance instance, VkPhysicalDevice pshysical_device,VkDevice device, VkQueue queue, std::uint32_t queue_family);
            
            void new_frame();
            void render();
            void show_demo_window();

            void backend_render_data(VkCommandBuffer cmd);

            void watch(uint32_t init_val, const std::vector<std::string>& vec, std::function<void(uint32_t slot, const std::string& s)>&& callback) { watch_initial_val = init_val; watch_vec = vec, watch_callback = callback; }

            void process();
        private:

            VkPipeline pipeline;
            VkPipelineLayout pipeline_layout;
            VkDescriptorPool pool;
            VkPipelineCache pipeline_cache;
            bool is_init = false;

            std::vector<std::string> watch_vec;
            uint32_t watch_initial_val = 0;
            std::function<void(uint32_t slot, const std::string& s)> watch_callback;
    };
};

