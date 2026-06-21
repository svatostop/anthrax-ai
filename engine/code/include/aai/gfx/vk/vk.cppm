module;
#include "aai/io/win_defines.h"
#include "aai/gfx/vk/backend/vk_defines.h"
#include <memory>
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
import std;

export {
   namespace vk {
       class base {
           public:
                void init(bool validate,  GLFWwindow* glfw_win, Display* di, Window w, std::shared_ptr<keeper::camera> c);
                
                bool begin_frame();
                void end_frame();
                void execute();

                void clean_rts() { render.clean_rts(dev.get_devices()); }
                void create_texture(const char* path, std::shared_ptr<rt::render_target> target);
                void create_model(const char* path, std::shared_ptr<model::base> m);
                void create_material(mat::materials& m) { pipe.create_material(dev.get_device(), m); } 
                void push_rq(const rq::data& r) { rq.push_back(r); }

                VkDeviceAddress get_buffer_address() { return gpu_mem.get_buffer_address(); }

                rt::render_target* get_last_target() { return render.get_last_target(); } 
                const rt::base::ref& get_attachment_ref(const rt::base::name r) { return render.get_attachment_ref(r); }
                
                vk::device::handlers get_devices() { return dev.get_devices(); }
                void wait_timeline() { frame.wait_timeline(dev.get_device(), dev.get_swapchain(), dev.get_queue(vk::queues::type::GRAPHICS)); }

                const glm::ivec2& get_window_size() { return window_size; }

                void set_camera(std::shared_ptr<keeper::camera> c) { cam = c; } 
           private:
                void init_rt_states();
                void on_resize();

                void set_debug_name(const std::string& name, uint64_t handle, VkObjectType type);
                void set_debug_render_pass_name(VkCommandBuffer cmd, const std::string& name);
                void unset_debug_render_pass_name(VkCommandBuffer cmd);
            
                std::deque<rq::data> rq;
                
                instance inst;
                device dev;
                frames frame;
                gpu_memory gpu_mem;
                pipeline pipe;

                renderer render;
                glm::ivec2 window_size;
                std::shared_ptr<keeper::camera> cam;
        };
   }
};

