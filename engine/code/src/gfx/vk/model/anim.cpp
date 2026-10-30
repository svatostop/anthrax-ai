module;
#include "aai/gfx/vk/model/model_types.h"

module aai.gfx.vk.model.anim;

import aai.gfx.debug;
static glm::vec4 sample_sampler(const model::animation::sampler& s, float t, bool is_rotation, const glm::mat4& model_matrix)
{
    if (s.inputs.empty()) return glm::vec4(0);
    if (t <= s.inputs.front()) return s.outputs.front();
    if (t >= s.inputs.back())  return s.outputs.back();

    std::vector<float>::const_iterator it = std::lower_bound(s.inputs.begin(), s.inputs.end(), t);
    const size_t k = std::distance(s.inputs.begin(), it);
    const float t0 = s.inputs[k - 1], t1 = s.inputs[k];
    const float a  = (t1 > t0) ? (t - t0) / (t1 - t0) : 0.0f;

    if (s.interpolation == model::animation::sampler::interpolation_t::step)
        return s.outputs[k - 1];

    if (is_rotation) {
        glm::quat q0(s.outputs[k - 1].w, s.outputs[k - 1].x, s.outputs[k - 1].y, s.outputs[k - 1].z);
        glm::quat q1(s.outputs[k].w, s.outputs[k].x, s.outputs[k].y, s.outputs[k].z);
        glm::quat q = glm::slerp(q0, q1, a);
        return glm::vec4(q.x, q.y, q.z, q.w);
    }
    
    return glm::mix(s.outputs[k - 1], s.outputs[k], a);
}

void anim::base::update(float dt, std::vector<glm::vec3>& translation, std::vector<glm::quat>& rotation, std::vector<glm::vec3>& scale)
{
    if (!raw_model || clip < 0 || clip >= static_cast<int32_t>(raw_model->info.clips.size())) 
        return;

    const auto& c = raw_model->info.clips[clip];

    time += dt * gfx::dbg::draw::get()->get_global_anim_mult();
    const float duration = c.end - c.start;
    if (duration > 0.0f) {
        if (loop) {
            time = c.start + std::fmod(time - c.start, duration);
            if (time < c.start) 
                time += duration;
        } 
        else {
            time = std::min(time, c.end);
        }
    }
    for (const auto& ch : c.channels) {
        if (ch.node_ind < 0 || ch.node_ind >= static_cast<int32_t>(raw_model->info.nodes.size())) 
            continue;

        const auto& s = c.samplers[ch.sampler_index];
        const bool is_rot = (ch.path == model::animation::channel::path_t::rotation);
        const glm::vec4 v = sample_sampler(s, time, is_rot, model_matrix);
        const int32_t i = ch.node_ind;

        switch (ch.path) {
            case model::animation::channel::path_t::translation:
                translation[i] = glm::vec3(v); 
                break;
            case model::animation::channel::path_t::rotation:
                rotation[i] = glm::quat(v.w, v.x, v.y, v.z); 
                break;
            case model::animation::channel::path_t::scale:
                scale[i] = glm::vec3(v); 
                break;
        }
    }
}
