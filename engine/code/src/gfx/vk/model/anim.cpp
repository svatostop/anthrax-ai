module;
#include "aai/gfx/vk/model/model_types.h"
#include <memory_resource>
#include <unistd.h>

module aai.gfx.vk.model.anim;

import aai.gfx.debug;
static glm::vec4 sample_sampler(const model::animation::sampler& s, float t, bool is_rotation, const glm::mat4& model_matrix)
{
    if (s.inputs.empty()) 
        return glm::vec4(0);
    if (t <= s.inputs.front()) 
        return s.outputs.front();
    if (t >= s.inputs.back())  
        return s.outputs.back();

    std::vector<float>::const_iterator it = std::lower_bound(s.inputs.begin(), s.inputs.end(), t);
    const size_t k = std::distance(s.inputs.begin(), it);
    const float t0 = s.inputs[k - 1], t1 = s.inputs[k];
    const float a  = (t1 > t0) ? (t - t0) / (t1 - t0) : 0.0f;

    if (is_rotation) {
        glm::quat q0(s.outputs[k - 1].w, s.outputs[k - 1].x, s.outputs[k - 1].y, s.outputs[k - 1].z);
        glm::quat q1(s.outputs[k].w, s.outputs[k].x, s.outputs[k].y, s.outputs[k].z);
        glm::quat q = glm::slerp(q0, q1, a);
        return glm::vec4(q.x, q.y, q.z, q.w);
    }
    
    return glm::mix(s.outputs[k - 1], s.outputs[k], a);
}

void anim::base::play(int32_t slot)
{
    if (slot < clips.size()) {
        int32_t fin_index = (clips[slot].clip >= 0 && clips[slot].clip < raw_model->info.clips.size()) ? clips[slot].clip : -1;
        clips[slot].clip = fin_index;
        clips[slot].time = (raw_model && fin_index >= 0) ? raw_model->info.clips[fin_index].start : 0.0f;
    }
    int active_clips = 0;
    for (int i = 0; i < clips.size(); i++) {
        if (clips[i].clip >= 0 && ++active_clips > 1)
            break;
    }
    has_bledning = active_clips > 1;
}

float safe_ratio_zero(float num, float den) {
    if (den != 0)
        return num / den;
    return 0;
}

bool anim::base::is_in_mask(anim::mask::val mask, const int32_t node_ind) {
    if (mask == mask::val::NONE)
        return false;
    static std::vector<std::string> upper_body_joints = {
        "spine_01", "spine_02", "spine_03", "neck_01", "head", "head_leaf",
        "clavicle_l", "upperarm_l", "lowerarm_l", "hand_l",
        "index_01_l", "index_02_l", "index_03_l", "index_04_leaf_l",
        "middle_01_l", "middle_02_l", "middle_03_l", "middle_04_leaf_l",
        "pinky_01_l", "pinky_02_l", "pinky_03_l", "pinky_04_leaf_l",
        "ring_01_l", "ring_02_l",  "ring_03_l", "ring_04_leaf_l",
        "thumb_01_l", "thumb_02_l", "thumb_03_l", "thumb_04_leaf_l",
        "clavicle_r", "upperarm_r", "lowerarm_r", "hand_r",
        "index_01_r", "index_02_r", "index_03_r", "index_04_leaf_r",
        "middle_01_r", "middle_02_r", "middle_03_r", "middle_04_leaf_r",
        "pinky_01_r", "pinky_02_r", "pinky_03_r", "pinky_04_leaf_r",
        "ring_01_r", "ring_02_r",  "ring_03_r",  "ring_04_leaf_r",
        "thumb_01_r", "thumb_02_r", "thumb_03_r", "thumb_04_leaf_r",
    };
    static std::vector<std::string> lower_body_joints = {
        "thigh_l", "calf_l", "foot_l", "ball_l", "ball_leaf_l",
        "thigh_r", "calf_r", "foot_r", "ball_r", "ball_leaf_r",
    };
    if (mask == mask::val::UPPER_BODY && std::find(upper_body_joints.begin(), upper_body_joints.end(), raw_model->info.nodes[node_ind].name ) != upper_body_joints.end()) {
        return true;
    }
    else if (mask == mask::val::LOWER_BODY && std::find(lower_body_joints.begin(), lower_body_joints.end(), raw_model->info.nodes[node_ind].name ) != lower_body_joints.end()) {
        return true;
    }

    return false;
}

void anim::base::update_clip(const model::animation::clip& c, clip_info& info, model::skeleton_state& state)
{
    const float duration = c.end - c.start;
    if (duration > 0.0f) {
        if (info.loop) {
            info.time = c.start + std::fmod(info.time - c.start, duration);
            if (info.time < c.start) 
                info.time += duration;
        } 
        else {
            info.time = std::min(info.time, c.end);
        }
    }
    for (const model::animation::channel& ch : c.channels) {
        if (ch.node_ind < 0 || ch.node_ind >= static_cast<int32_t>(raw_model->info.nodes.size())) 
            continue;

        const int32_t i = ch.node_ind;
        float weight = info.final_weight * mask_weight[i];
        if (weight <= 0.01 && is_in_mask(info.mask_type, i)) {
            // continue;
            weight = 0;// info.mask_weight;
        }

        const auto& s = c.samplers[ch.sampler_index];
        const bool is_rot = (ch.path == model::animation::channel::path_t::rotation);
        const glm::vec4 v = sample_sampler(s, info.time, is_rot, model_matrix);
        
        switch (ch.path) {
            case model::animation::channel::path_t::translation: {
                state.translation[i] += weight * glm::vec3(v); 
                state.weights_translation[i] += weight ;//* glm::vec3(v); 
                break;
                                                                 }
            case model::animation::channel::path_t::rotation: {
                glm::quat def_quat = raw_model->info.nodes[i].default_rotation;
                glm::quat anim_quat = glm::quat(v.w, v.x, v.y, v.z);
                if (dot(def_quat, anim_quat) < 0)
                    anim_quat = -anim_quat;
                state.rotation[i] += weight * anim_quat;  
                state.weights_rotation[i] += weight ;
                break;
            }
            case model::animation::channel::path_t::scale:
                state.scale[i] += weight * glm::vec3(v); 
                state.weights_scale[i] += weight ;
                break;
        }
    }

}

void anim::base::update(float dt, model::skeleton_state& output_states)
{
    if (!raw_model) 
        return;

    output_states.clear();
    
    switch (blend_method) {
        case blend_method::val::FACTOR: {
            float weights = 0.0f;
            for (int i = 0; i < clips.size(); i++) {
                float w1 = 1.0f;
                if (!clips[i].mults[0].is_blank) {
                    w1 = clips[i].mults[0].is_reversed ? 1.0f - blend_factors[clips[i].mults[0].blend_factor_ind] : blend_factors[clips[i].mults[0].blend_factor_ind];
                }
                float w2 =1.0f;
                if (!clips[i].mults[1].is_blank) {
                    w2 = clips[i].mults[1].is_reversed ? 1.0f - blend_factors[clips[i].mults[1].blend_factor_ind] : blend_factors[clips[i].mults[1].blend_factor_ind];
                }               
                clips[i].final_weight = w1 * w2; 
            }
            break;
        }
        default:
        case blend_method::val::WEIGHT_SUM: {
            // printf("-------------\n");
            float weight_sum = 0.0f;
            for (int i = 0; i < clips.size(); i++) {
                weight_sum += clips[i].weight_to_sum;
            }
            for (int i = 0; i < clips.size(); i++) {
                clips[i].final_weight = safe_ratio_zero(clips[i].weight_to_sum, weight_sum);
                // printf("fin weight: %d|%f\n", i, clips[i].final_weight);
            }
            break;
        }

    }
       
    float speed = 0.0f;
    for (int i = 0; i < clips.size(); i++) {
        if (clips[i].clip < 0) {
            continue;
        } 
        const model::animation::clip& c = raw_model->info.clips[clips[i].clip];
        const float duration = c.end - c.start;
        speed += safe_ratio_zero(1.0f, duration) * clips[i].final_weight;  
    }
    synced_time += speed * dt * gfx::dbg::draw::get()->get_global_anim_mult();
    synced_time = std::fmod(synced_time, 1);
    for (int i = 0; i < clips.size(); i++) {
        if (clips[i].clip < 0) {
            continue;
        } 

        const model::animation::clip& c = raw_model->info.clips[clips[i].clip];
        const float duration = c.end - c.start;
        clips[i].time = (synced_time * duration) + clips[i].offset_time;

        update_clip(c, clips[i], output_states);
    }
    for (size_t i = 0; i < raw_model->info.nodes.size(); ++i) {
        const auto& n = raw_model->info.nodes[i];
        if (output_states.weights_translation[i] == 0.0) {
            output_states.translation[i] = n.default_translation;
        }
        else {
            output_states.translation[i] *= (1 / output_states.weights_translation[i]);
        }
        if (output_states.weights_rotation[i] == 0)
            output_states.rotation[i] = n.default_rotation;
        else 
            output_states.rotation[i] *= (1 / output_states.weights_rotation[i]);
        
        if (output_states.weights_scale[i] == 0)
            output_states.scale[i] = n.default_scale;
        else 
            output_states.scale[i] *= (1 / output_states.weights_scale[i]);

        output_states.weights_translation[i] = 1.0f;
        output_states.weights_scale[i] = 1.0f;
        output_states.weights_rotation[i] = 1.0f;
    }
}

