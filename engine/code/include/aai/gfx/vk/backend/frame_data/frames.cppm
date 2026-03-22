module;
#include <cstdint>
#include "aai/gfx/vk/backend/vk_defines.h"

export module aai.gfx.vk.frames;

export import aai.gfx.vk.frames.cmd;
export import aai.gfx.vk.frames.sync;
export {
   namespace vk {
       class frames {
           public:
                void init(VkDevice dev, const uint32_t graphics_index);
           private:
                command_buffer cmd;
                synchronization sync;
        };
   }
};
