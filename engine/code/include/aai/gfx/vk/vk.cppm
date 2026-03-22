module;
#include "aai/io/win_defines.h"

export module aai.gfx.vk;
export import aai.gfx.vk.instance;
export import aai.gfx.vk.device;
export import aai.gfx.vk.frames;
export {
   namespace vk {
       class base {
           public:
                void init(bool validate, Display* di, Window w);
                
                void create_texture(const char* path);
           private:
                instance inst;
                device dev;
                frames frame;
        };
   }
};

