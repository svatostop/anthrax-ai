module;
#include "aai/io/win_defines.h"

export module aai.gfx.vk;
export import aai.gfx.vk.instance;
export import aai.gfx.vk.device;
export import aai.gfx.vk.frames;
export import aai.gfx.vk.gpu_memory;
export import aai.gfx.vk.pipeline;
export import aai.gfx.vk.rq;
import aai.gfx.vk.rt.helper;
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
                void create_material(mat::materials& m) { pipe.create_material(dev.get_device(), m); } 
                void set_rq(const rq::data& r) { rq = r; }

                const rt::base::ref& get_attachment_ref(const rt::base::name r) { return rts.get_ref(r); }
           private:
                void render_block();
                void start_render(const rt::base::ref& attachment_ref);
                void end_render();
                void draw();

                void submit(std::function<void(VkCommandBuffer cmd)>&& func);

                void set_debug_name(const std::string& name, uint64_t handle, VkObjectType type);
                void set_debug_render_pass_name(VkCommandBuffer cmd, const std::string& name);
                void unset_debug_render_pass_name(VkCommandBuffer cmd);
            
                PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR{VK_NULL_HANDLE};
        	    PFN_vkCmdEndRenderingKHR   vkCmdEndRenderingKHR{VK_NULL_HANDLE};

                rq::data rq;
                rt::base rts; 
                    
                instance inst;
                device dev;
                frames frame;
                gpu_memory gpu_mem;
                pipeline pipe;
        };
   }
};

