module;
#include "aai/utils/lookup_table.h"
#include "aai/gfx/vk/model/model_types.h"
#include <cstdint>

export module aai.gfx.vk.model.anim;

export import aai.gfx.vk.model;

export {
    namespace anim {
        namespace mask {
#define ANIM_MASK_LOOKUP(X) \
            X(NONE, "none") \
            X(UPPER_BODY, "upper body") \
            X(LOWER_BODY, "lower body") \
            X(SIZE, "mask size")
DECLARE_LOOKUP_TABLE(ANIM_MASK_LOOKUP, val)
        }
        namespace blend_method {
#define BLEND_METHOD_LOOKUP(X) \
            X(WEIGHT_SUM, "sum of weight") \
            X(FACTOR, "blend factor") \
            X(SIZE, "mask size")
DECLARE_LOOKUP_TABLE(BLEND_METHOD_LOOKUP, val)
        }
        class base {
            public:
                base() {
                    clips.reserve(16);
                    blend_factors.reserve(16);
                }
                void play(int32_t slot);
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
                void remove_clip(uint32_t ind) { if (ind >= 0 && ind < clips.size()) clips.erase(clips.begin() + ind); }

                void set_raw_model(const std::shared_ptr<model::base> a) { raw_model = a; }
                bool is_model_valid() { return raw_model != nullptr; }

                const std::string& get_current_clip(uint32_t slot) { return  raw_model->info.clips[clips[slot].clip].name; }
                const std::vector<std::string>& get_clip_names() { return  raw_model->info.clip_names;}

                model::skeleton_state& get_output_states() { return output_states; }
                struct blend_multipliers {
                    bool is_reversed = false;
                    bool is_blank = false;
                    int32_t blend_factor_ind = 0;
                };
                struct clip_info {
                    int32_t clip = -1;
                    float time = 0.0f;
                    bool loop  = true;
                    float offset_time = 0.0f;
                    float weight_to_sum = 1.0f;
                    float final_weight = 1.0f;

                    mask::val mask_type = mask::val::NONE;
                    blend_multipliers mults[2];
                };
                using clip_info_vec = std::vector<clip_info>;
                clip_info_vec clips;
                clip_info& add_clip() { return clips.emplace_back(); }

                blend_method::val blend_method = blend_method::val::WEIGHT_SUM;

                std::vector<float>& get_blend_factors() { return blend_factors; }
                void add_blend_factor() { blend_factors.push_back(0.0f); }
            private:
                bool is_in_mask(anim::mask::val mask, const int32_t node_ind);
                void update_clip(const model::animation::clip& c, clip_info& info, model::skeleton_state& state);
                glm::mat4 model_matrix = glm::mat4(1.0f);
                std::shared_ptr<model::base> raw_model = nullptr;
                model::skeleton_state output_states;
            
                std::vector<float> blend_factors;
                float synced_time = 0.0f;
                bool has_bledning = false;
        };
    }
};
