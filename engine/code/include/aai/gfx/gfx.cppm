module;
#include "aai/io/win_defines.h"

export module aai.gfx;

export import aai.gfx.vk;
import std;
export {
    namespace gfx {
        class base {
            public:
                void init(Display* di, Window w);

                void create_texture(const char* path);
            private:
                vk::base vk;
        };
    }
};
