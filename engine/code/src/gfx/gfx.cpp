#include "aai/io/win_defines.h"

import aai.gfx;
import std;

void gfx::base::init(Display* di, Window w)
{
    vk.init(true, di, w);
}

void gfx::base::create_texture(const char* path)
{
    auto future = asset_mng.load_async(path, [&](const std::string&) {  
        //vk.create_texture(path);
        return nullptr;
    });

    // material_mng.load_async(shaders);
    // rq r = {
    //     .add_resource(handle_id),
    //     .add_material(material_mng.get(TETS)),
    //
    // }
}
