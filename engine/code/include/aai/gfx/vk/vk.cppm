module;
#include "aai/io/win_defines.h"
#include "aai/gfx/vk/backend/vk_defines.h"
#include <memory>
#include "aai/gfx/vk/model/model_types.h"
#include <stdio.h>

export module aai.gfx.vk;
export import aai.keeper.camera;
export import aai.gfx.vk.instance;
export import aai.gfx.vk.device;
export import aai.gfx.vk.frames;
export import aai.gfx.vk.gpu_memory;
export import aai.gfx.vk.pipeline;
export import aai.gfx.vk.rq;
export import aai.gfx.vk.renderer;
import aai.gfx.vk.device.helper;
export import aai.gfx.vk.model;
export import aai.gfx.vk.model.inst;
export import aai.editor;
import std;

export {
   namespace vk {
       class base {
           public:
                void init(bool validate,  GLFWwindow* glfw_win, Display* di, Window w, std::shared_ptr<keeper::camera> c);
                
                bool begin_frame();
                void end_frame();
                void execute(float delta_ms);

                void clean_rts() { render.clean_rts(dev.get_devices()); }
                void create_texture(const std::string& path, std::shared_ptr<rt::render_target> target);
                void create_model(const char* path, std::shared_ptr<model::base> m);
                uint32_t create_material(mat::materials& m, const std::string& name) { return pipe.create_material(dev.get_device(), m, name); } 
                void push_static_render_data(rq::data&& r) { rq.push_back(std::move(r)); }
                void push_render_data(rq::data&& r) { rq_test.push_back(std::move(r)); }

                rt::render_target* get_last_target() { return render.get_last_target(); } 
                const rt::base::ref& get_attachment_ref(const rt::name::val r) { return render.get_attachment_ref(r); }
                
                vk::device::handlers get_devices() { return dev.get_devices(); }
                void wait_timeline() { frame.wait_timeline(dev.get_device(), dev.get_swapchain(), dev.get_queue(vk::queues::type::GRAPHICS)); }

                const glm::ivec2& get_window_size() { return window_size; }

                void set_camera(std::shared_ptr<keeper::camera> c) { cam = c; }

                void init_editor(GLFWwindow* window, aai::editor& editor);

                uint32_t get_skin_id() { return gpu_mem.get_skin_id(); }
                std::vector<VkDeviceAddress>& get_skin_gpu_address(const std::shared_ptr<model::skin_groups> group) { return gpu_mem.get_skin_gpu_address(group, dev.get_devices()); }
           private:
                void init_rt_states();
                void on_resize();

                void set_debug_name(const std::string& name, uint64_t handle, VkObjectType type);
                void set_debug_render_pass_name(VkCommandBuffer cmd, const std::string& name);
                void unset_debug_render_pass_name(VkCommandBuffer cmd);
            
                std::deque<rq::data> rq;
                std::deque<rq::data> rq_test;
                
                instance inst;
                device dev;
                frames frame;
                gpu_memory gpu_mem;
                pipeline pipe;

                renderer render;
                glm::ivec2 window_size;
                std::shared_ptr<keeper::camera> cam;

                std::function<void(VkCommandBuffer cmd)> editor_render_callback;
        };
   }
};

