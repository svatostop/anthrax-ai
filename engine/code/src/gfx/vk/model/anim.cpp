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
void anim::base::play(int32_t slot, int32_t clip_index, bool l)
{
    if (slot < MAX_BLEND_SIZE) {
        int32_t fin_index = (clip_index >= 0 && clip_index < raw_model->info.clips.size()) ? clip_index : -1;
        clips[slot].clip = fin_index;
        clips[slot].loop = l;
        clips[slot].time = (raw_model && fin_index >= 0) ? raw_model->info.clips[fin_index].start : 0.0f;
    }
    int active_clips = 0;
    for (int i = 0; i < MAX_BLEND_SIZE; i++) {
        if (clips[i].clip >= 0 && ++active_clips > 1)
            break;
    }
    has_bledning = active_clips > 1;
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

        const auto& s = c.samplers[ch.sampler_index];
        const bool is_rot = (ch.path == model::animation::channel::path_t::rotation);
        const glm::vec4 v = sample_sampler(s, info.time, is_rot, model_matrix);
        const int32_t i = ch.node_ind;

        switch (ch.path) {
            case model::animation::channel::path_t::translation:
                state.translation[i] += info.weight * glm::vec3(v); 
                break;
            case model::animation::channel::path_t::rotation: {
                glm::quat def_quat = raw_model->info.nodes[i].default_rotation;
                glm::quat anim_quat = glm::quat(v.w, v.x, v.y, v.z);
                if (dot(def_quat, anim_quat) < 0)
                    anim_quat = -anim_quat;
                state.rotation[i] += info.weight * anim_quat;  
                break;
            }
            case model::animation::channel::path_t::scale:
                state.scale[i] += info.weight * glm::vec3(v); 
                break;
        }
    }

}

float safe_ratio_zero(float num, float den) {
    if (den != 0)
        return num / den;
    return 0;
}

void anim::base::update(float dt, model::skeleton_state& output_states)
{
    if (!raw_model) 
        return;

    output_states.clear();
    
    float blend_factor = gfx::dbg::draw::get()->get_blend_factor();
    if (has_bledning) {
        clips[0].weight = 1.0 - blend_factor;
        clips[1].weight = blend_factor;
        clips[1].offset_time = 0.7f;
    }

    float speed = 0.0f;
    for (int i = 0; i < MAX_BLEND_SIZE; i++) {
        if (clips[i].clip < 0) {
            continue;
        } 
        const model::animation::clip& c = raw_model->info.clips[clips[i].clip];
        const float duration = c.end - c.start;
        speed += safe_ratio_zero(1.0f, duration) * clips[i].weight;  
    }
    synced_time += speed * dt * gfx::dbg::draw::get()->get_global_anim_mult();
    synced_time = std::fmod(synced_time, 1);
    for (int i = 0; i < MAX_BLEND_SIZE; i++) {
        if (clips[i].clip < 0) {
            continue;
        } 
        if (has_bledning && ((blend_factor == 0.0f && i != 0) || (blend_factor == 1.0f && i != 1))) {
            continue;
        }
        const model::animation::clip& c = raw_model->info.clips[clips[i].clip];
        const float duration = c.end - c.start;
        clips[i].time = (synced_time * duration) + clips[i].offset_time;

        update_clip(c, clips[i], output_states);
    }
    for (size_t i = 0; i < raw_model->info.nodes.size(); ++i) {
        const auto& n = raw_model->info.nodes[i];
        if (output_states.translation[i] == glm::vec3(0))
            output_states.translation[i] = n.default_translation;
        if (output_states.rotation[i] == glm::quat(0,0,0,0))
            output_states.rotation[i] = n.default_rotation;
        if (output_states.scale[i] == glm::vec3(0))
            output_states.scale[i] = n.default_scale;
    }
}

