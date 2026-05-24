module;
#include "aai/io/win_defines.h"

export module aai.gfx;

export import aai.gfx.vk;
export import aai.gfx.assets;
export import aai.gfx.vk.rt;
export import aai.gfx.materials;
import std;
export {
    namespace gfx {
        class base {
            public:
                void init(Display* di, Window w);
                
                void run();
                void populate();
                uint32_t create_texture(const char* path);

                void clean() { vk.wait_timeline(); clean_resources(); }
            private:
                void clean_resources();
                vk::base vk;
                assets::base<rt::render_target> asset_mng;
                mat::materials material_pallet;
        };
    }
};
