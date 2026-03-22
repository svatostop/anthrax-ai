#include "aai/gfx/vk/backend/vk_defines.h"
#include <cstdint>
import aai.gfx.vk.frames;

void vk::frames::init(VkDevice dev, const uint32_t graphics_index)
{
    cmd.init(dev, graphics_index); 
    sync.init(dev);
}
