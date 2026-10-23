module;
#include "aai/gfx/vk/model/model_types.h"
#include <iterator>

export module aai.gfx.vk.model.anim;

export import aai.gfx.vk.model;

export {
    namespace anim {
        class base {
            public:
                void play(int32_t clip_index, bool l = true) {
                    clip = clip_index;
                    loop = l;
                    time = (raw_model && clip >= 0) ? raw_model->info.clips[clip].start : 0.0f;
                }
                void update(float dt, std::vector<glm::vec3>& translation, std::vector<glm::quat>& rotation, std::vector<glm::vec3>& scale);

                uint32_t find_clip_by_name(const std::string& name) {
                    if (!raw_model)
                        return 0;
                    std::vector<std::string>::iterator it = std::find(raw_model->info.clip_names.begin(), raw_model->info.clip_names.end(), name);
                    if (it != raw_model->info.clip_names.end())
                        return (std::distance(raw_model->info.clip_names.begin(), it));
                    return 0;
                }
 
                void set_raw_model(const std::shared_ptr<model::base> a) { raw_model = a; }

                const std::string& get_current_clip() { return  raw_model->info.clips[clip].name; }
                const std::vector<std::string>& get_clip_names() { return  raw_model->info.clip_names;}

                int32_t clip = -1;
                float time = 0.0f;
                float speed = 1.0f;
                bool loop  = true;
            private:
                std::shared_ptr<model::base> raw_model = nullptr;
        };
    }
};
