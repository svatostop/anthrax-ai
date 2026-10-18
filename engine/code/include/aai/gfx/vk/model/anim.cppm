module;
#include "aai/gfx/vk/model/model_types.h"

export module aai.gfx.vk.model.anim;

export import aai.gfx.vk.model;

export {
    namespace anim {
        class base {
            public:
                void play(int32_t clip_index, bool l = true) {
                    clip = clip_index;
                    loop = l;
                    time = (asset && clip >= 0) ? asset->info.clips[clip].start : 0.0f;
                }
                void update(float dt, std::vector<glm::vec3>& translation, std::vector<glm::quat>& rotation, std::vector<glm::vec3>& scale);

                void set_asset(const model::base* a) { asset = a; }
                int32_t clip = -1;
                float time = 0.0f;
                float speed = 1.0f;
                bool loop  = true;
            private:
                const model::base* asset = nullptr;
        };
    }
};
