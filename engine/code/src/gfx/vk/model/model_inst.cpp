module;
#include "aai/gfx/vk/loaders/model_loader.h"

module aai.gfx.vk.model.inst;

void model::inst::init(const model::base* m)
{
    asset = m;
    const size_t n = m->info.nodes.size();
    translation.resize(n);
    rotation.resize(n);
    scale.resize(n);
    matrix.resize(n);
    local.resize(n);
    global.resize(n);

    for (size_t i = 0; i < n; ++i) {
        const auto& n = m->info.nodes[i];
        translation[i] = n.default_translation;
        rotation[i] = n.default_rotation;
        scale[i] = n.default_scale;
        matrix[i] = n.default_matrix;
    }

    skin_data.resize(m->info.skeletons.size());
    anim.set_asset(m);
    update(0.0f);
}

void model::inst::update(float dt)
{
    if (anim.clip >= 0)
        anim.update(dt, translation, rotation, scale);

    for (size_t i = 0; i < asset->info.nodes.size(); ++i) {
        const auto& n = asset->info.nodes[i];
        if (n.use_matrix)
            local[i] = matrix[i];
        else 
            local[i] = glm::translate(glm::mat4(1.0f), translation[i]) * glm::mat4_cast(rotation[i]) * glm::scale(glm::mat4(1.0f), scale[i]);
    }
    for (int32_t r : asset->info.root_nodes) 
        compute_globals(r);

    for (size_t i = 0; i < asset->info.nodes.size(); ++i) {
        const auto& n = asset->info.nodes[i];
        if (n.skin_id < 0) continue;
        const auto& skelets = asset->info.skeletons[n.skin_id];
        auto& data = skin_data[n.skin_id];
        data.resize(skelets.bones.size());
        const glm::mat4 inv_mat = glm::inverse(global[i]);
        for (size_t j = 0; j < skelets.bones.size(); ++j)
            data[j] = inv_mat * global[skelets.bones[j]] * skelets.inv_matrices[j];
    }
}

void model::inst::compute_globals(int32_t i)
{
    const auto& n = asset->info.nodes[i];
    const glm::mat4 parent = (n.parent >= 0) ? global[n.parent] : world;
    global[i] = parent * local[i];
    for (int32_t c : n.children)
        compute_globals(c);
}

