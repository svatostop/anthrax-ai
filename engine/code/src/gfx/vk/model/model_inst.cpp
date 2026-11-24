module;
#include "aai/gfx/vk/model/model_types.h"
#include <memory>

module aai.gfx.vk.model.inst;
import aai.gfx.debug;

void model::inst::init(const std::shared_ptr<model::base> m, uint32_t skin_id)
{
    raw_model = m;

    const size_t n = m->info.nodes.size();
    // states.translation.resize(n);
    // states.rotation.resize(n);
    // states.scale.resize(n);
    states.resize(n);
    gltf_matrix.resize(n);
    local.resize(n, glm::mat4(1.0f));
    global.resize(n, glm::mat4(1.0f));

    for (size_t i = 0; i < n; ++i) {
        const auto& n = m->info.nodes[i];
        states.translation[i] = n.default_translation;
        states.rotation[i] = n.default_rotation;
        states.scale[i] = n.default_scale;
        gltf_matrix[i] = n.default_matrix;
    }

    skin_group = std::make_shared<model::skin_groups>();
    skin_group->skin_id = skin_id;
    if (skin_id > 0)
        skin_group->d.resize(m->info.skeletons.size());
    anim.set_raw_model(m);
    // if (raw_model)
    //     states.resize(raw_model->info.nodes.size());
    update(0.0f);

    anim.set_model_matrix(model_matrix);
   
    if (has_anim()) {
        anim.add_blend_factor();
        float blend_factor = 0.0f;
        auto& clip1 = anim.add_clip();
        clip1.clip = 0;
        clip1.loop = true;
        clip1.weight_to_sum = 1.0f - blend_factor;
        clip1.offset_time = 0.7;
        anim.play(0);
        auto& clip2 = anim.add_clip();
        clip2.clip = 1;
        clip2.loop = true;
        clip2.weight_to_sum = blend_factor;
        anim.play(1);
    }
}

void model::inst::update(float dt)
{
    if (!has_anim())
        return;
    anim.update(dt, states);

    for (size_t i = 0; i < raw_model->info.nodes.size(); ++i) {
        const auto& n = raw_model->info.nodes[i];
        local[i] = glm::translate(glm::mat4(1.0f), states.translation[i]) * glm::mat4_cast(states.rotation[i]) * glm::scale(glm::mat4(1.0f), states.scale[i]) * gltf_matrix[i];
    }
    for (int32_t r : raw_model->info.root_nodes) 
        compute_globals(r);

    for (size_t i = 0; i < raw_model->info.nodes.size(); ++i) {
        const types::node& n = raw_model->info.nodes[i];
        if (n.skin_id < 0 || n.skin_id > skin_group->d.size() || skin_group->d.empty()) continue;
        const types::skeleton& skelets = raw_model->info.skeletons[n.skin_id];
        std::vector<glm::mat4>& data = skin_group->d[n.skin_id];
        if (data.size() != skelets.joints.size())
            data.resize(skelets.joints.size());
        // const glm::mat4 inv_mat = glm::inverse(global[i]);
        for (size_t j = 0; j < skelets.joints.size(); ++j) {
            data[j] = global[skelets.joints[j]] * skelets.inv_matrices[j];

            if (gfx::dbg::draw::get()->enabled()) {
                int32_t parent = raw_model->info.nodes[skelets.joints[j]].parent;
                if (parent < 0) continue;
                glm::vec3 a = glm::vec3(global[parent][3]);
                if (a.x == 0 && a.y == 0 && a.z == 0)
                    continue;
                glm::vec3 b = glm::vec3(global[skelets.joints[j]][3]);
                gfx::dbg::draw::get()->submit(gfx::dbg::primitive_t::LINE, a, b, model_matrix);
                gfx::dbg::draw::get()->submit(gfx::dbg::primitive_t::POINT, a, model_matrix);
            }
        }
    }

}

void model::inst::compute_globals(int32_t i)
{
    const auto& n = raw_model->info.nodes[i];
    const glm::mat4 parent = (n.parent >= 0) ? global[n.parent] : world;
    global[i] = parent * local[i];
    for (int32_t c : n.children)
        compute_globals(c);
}

