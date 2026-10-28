module;
#include "aai/gfx/vk/loaders/model_loader.h"
#include "aai/gfx/vk/model/model_types.h"
#include <memory>

module aai.gfx.vk.model.inst;

void model::inst::init(const std::shared_ptr<model::base> m, uint32_t skin_id)
{
    raw_model = m;

    const size_t n = m->info.nodes.size();
    translation.resize(n);
    rotation.resize(n);
    scale.resize(n);
    matrix.resize(n);
    local.resize(n);
    global.resize(n, glm::mat4(1.0f));

    for (size_t i = 0; i < n; ++i) {
        const auto& n = m->info.nodes[i];
        translation[i] = n.default_translation;
        rotation[i] = n.default_rotation;
        scale[i] = n.default_scale;
        matrix[i] = n.default_matrix;
        global[i] = glm::mat4(1.0f);
    }

    skin_group = std::make_shared<model::skin_groups>();
    skin_group->skin_id = skin_id;
    if (skin_id > 0)
        skin_group->d.resize(m->info.skeletons.size());
    anim.set_raw_model(m);
    update(0.0f);
   
    if (has_anim()) {
        anim.play(0);
    }
}

void model::inst::update(float dt)
{
    if (!has_anim())
        return;
    if (anim.clip >= 0)
        anim.update(dt, translation, rotation, scale);

    for (size_t i = 0; i < raw_model->info.nodes.size(); ++i) {
        const auto& n = raw_model->info.nodes[i];
        if (n.use_matrix)
            local[i] = matrix[i];
        else 
            local[i] = glm::translate(glm::mat4(1.0f), translation[i]) * glm::mat4_cast(rotation[i]) * glm::scale(glm::mat4(1.0f), scale[i]) * matrix[i];
    }
    for (int32_t r : raw_model->info.root_nodes) 
        compute_globals(r);

    for (size_t i = 0; i < raw_model->info.nodes.size(); ++i) {
        const auto& n = raw_model->info.nodes[i];
        if (n.skin_id < 0 || n.skin_id > skin_group->d.size() || skin_group->d.empty()) continue;
        const auto& skelets = raw_model->info.skeletons[n.skin_id];
        auto& data = skin_group->d[n.skin_id];
        if (data.size() != skelets.bones.size())
            data.resize(skelets.bones.size());
        const glm::mat4 inv_mat = glm::inverse(global[i]);
        for (size_t j = 0; j < skelets.bones.size(); ++j) {
            data[j] = global[skelets.bones[j]] * skelets.inv_matrices[j];
            data[j] = inv_mat * data[j];
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

