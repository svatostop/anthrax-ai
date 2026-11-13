module;
#include "aai/gfx/vk/model/model_types.h"
#include <iterator>

export module aai.gfx.vk.model.anim;

export import aai.gfx.vk.model;

export {
    namespace anim {
        class base {
            public:
                base() {
                }
                void play(int32_t slot, int32_t clip_index, bool l = true);
                void set_model_matrix(const glm::mat4& m) { model_matrix = m; }
                void update(float dt, model::skeleton_state& output_states);

                uint32_t find_clip_by_name(const std::string& name) {
                    if (!raw_model)
                        return 0;
                    std::vector<std::string>::iterator it = std::find(raw_model->info.clip_names.begin(), raw_model->info.clip_names.end(), name);
                    if (it != raw_model->info.clip_names.end())
                        return (std::distance(raw_model->info.clip_names.begin(), it));
                    return 0;
                }
 
                void set_raw_model(const std::shared_ptr<model::base> a) { raw_model = a; }

                const std::string& get_current_clip(uint32_t slot) { return  raw_model->info.clips[clips[slot].clip].name; }
                const std::vector<std::string>& get_clip_names() { return  raw_model->info.clip_names;}

                model::skeleton_state& get_output_states() { return output_states; }
                struct clip_info {
                    int32_t clip = -1;
                    float time = 0.0f;
                    bool loop  = true;
                    float weight = 1.0f;
                    float offset_time = 0.0f;
                };
                clip_info clips[MAX_BLEND_SIZE];
            private:
                void update_clip(const model::animation::clip& c, clip_info& info, model::skeleton_state& state);
                glm::mat4 model_matrix = glm::mat4(1.0f);
                std::shared_ptr<model::base> raw_model = nullptr;
                model::skeleton_state output_states;

                float synced_time = 0.0f;
                bool has_bledning = false;
        };
    }
};
