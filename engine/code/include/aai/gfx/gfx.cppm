module;
#include "aai/io/win_defines.h"

export module aai.gfx;

export import aai.gfx.vk;
export {
    namespace gfx {
        class base {
            public:
                void init(Display* di, Window w);
            private:
                vk::base vk;
        };
    }
};
