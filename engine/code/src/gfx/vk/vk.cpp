#include "aai/io/win_defines.h"

import aai.gfx.vk;

void vk::base::init(bool validate, Display* di, Window w)
{
    inst.init(validate);
#ifdef AAI_LINUX
    dev.init_linux_surface(inst.get_instance(), di, w);
#else 
    dev.init_windows_surface();
#endif
    dev.init(validate, inst.get_layers());
}

