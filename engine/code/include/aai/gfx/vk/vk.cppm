module;
#include "aai/io/win_defines.h"

export module aai.gfx.vk;
export import aai.gfx.vk.instance;
export import aai.gfx.vk.device;
export import aai.gfx.vk.frames;
import std;

export {
   namespace vk {
       class base {
           public:
                void init(bool validate, Display* di, Window w);
                
                void create_texture(const char* path);
           private:
                void submit(std::function<void(VkCommandBuffer cmd)>&& func);

                void set_debug_name(const std::string& name, uint64_t handle, VkObjectType type);
                void set_debug_render_pass_name(VkCommandBuffer cmd, const std::string& name);
                void unset_debug_render_pass_name(VkCommandBuffer cmd);

                instance inst;
                device dev;
                frames frame;
        };
   }
};

