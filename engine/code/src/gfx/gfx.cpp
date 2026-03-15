#include "aai/io/win_defines.h"

import aai.gfx;

void gfx::base::init(Display* di, Window w)
{
    vk.init(true, di, w);
}
