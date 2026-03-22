#include "aai/io/win_defines.h"

import aai.gfx;
// import std;

void gfx::base::init(Display* di, Window w)
{
    vk.init(true, di, w);

    // rts.create_texture("bla", vk.get_devices())
}

void gfx::base::create_texture(const char* path)
{
    vk.create_texture(path);   
}
