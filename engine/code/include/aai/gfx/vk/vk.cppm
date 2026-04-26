module;
#include "aai/io/win_defines.h"

export module aai.gfx.vk;
export import aai.gfx.vk.instance;
export import aai.gfx.vk.device;
export import aai.gfx.vk.frames;
export import aai.gfx.vk.gpu_memory;
export import aai.gfx.vk.pipeline;
export import aai.gfx.vk.rq;
export import aai.gfx.attachments;
import aai.gfx.vk.rt;
import std;

export {
   namespace vk {
       class base {
           public:
                void init(bool validate, Display* di, Window w);
                
                bool begin_frame();
                void end_frame();
                void render();

                void create_texture(const char* path);
                void create_material(mat::materials& m, const rt::attachments::ref& attachments) { pipe.create_material(dev.get_device(), m, attachments); } 
                void set_rq(const rq::data& r) { rq = r; }
           private:
                void render_block();
                void start_render(rt::render_target* target);
                void end_render();
                void draw();

                void submit(std::function<void(VkCommandBuffer cmd)>&& func);

                void set_debug_name(const std::string& name, uint64_t handle, VkObjectType type);
                void set_debug_render_pass_name(VkCommandBuffer cmd, const std::string& name);
                void unset_debug_render_pass_name(VkCommandBuffer cmd);
            
                PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR{VK_NULL_HANDLE};
        	    PFN_vkCmdEndRenderingKHR   vkCmdEndRenderingKHR{VK_NULL_HANDLE};

                rq::data rq;
                instance inst;
                device dev;
                frames frame;
                gpu_memory gpu_mem;
                pipeline pipe;
        };
   }
};

