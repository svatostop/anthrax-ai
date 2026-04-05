module;
#include "aai/gfx/vk/backend/vk_defines.h"

export module aai.gfx.vk.pipeline;

export import aai.gfx.materials;
export {
    namespace vk {
        class pipeline {
            public:
                void create_material(mat::materials& m);
            private:
                
        };
    }
};
