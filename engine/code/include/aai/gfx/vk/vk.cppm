module;
#include "aai/io/win_defines.h"

export module aai.gfx.vk;
export import aai.gfx.vk.instance;
export import aai.gfx.vk.device;
export {
   namespace vk {
       class base {
           public:
                void init(bool validate, Display* di, Window w);
           private:
                instance inst;
                device dev;
       };
   }
};

