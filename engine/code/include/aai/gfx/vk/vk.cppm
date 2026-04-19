module;
#include "aai/io/win_defines.h"

export module aai.gfx.vk;
export import aai.gfx.vk.instance;
export import aai.gfx.vk.device;
export import aai.gfx.vk.frames;
export import aai.gfx.vk.gpu_memory;
export import aai.gfx.vk.pipeline;
export import aai.gfx.vk.rq;
export import aai.gfx.attachment_ref;
import aai.gfx.vk.rt;
import std;

export {
   namespace vk {
       class base {
           public:
                void init(bool validate, Display* di, Window w);
                
                bool begin_frame();
                void end_frame();

                void create_texture(const char* path);
                void create_material(mat::materials& m, const rt::attachment_ref::info& attachments) { pipe.create_material(dev.get_device(), m, attachments); } 
                void set_rq(const rq::data& r) { rq = r; }
           private:
                void submit(std::function<void(VkCommandBuffer cmd)>&& func);

                void set_debug_name(const std::string& name, uint64_t handle, VkObjectType type);
                void set_debug_render_pass_name(VkCommandBuffer cmd, const std::string& name);
                void unset_debug_render_pass_name(VkCommandBuffer cmd);
            
                rq::data rq;
                instance inst;
                device dev;
                frames frame;
                gpu_memory gpu_mem;
                pipeline pipe;

                rt::render_target* main_rt;
        };
   }
};

