module;
#include "aai/gfx/vk/loaders/model_loader.h"

module aai.gfx.vk.model.inst;

void model::inst::init(const std::shared_ptr<model::base> m, vk::device::handlers devices)
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

    skin_data.resize(m->info.skeletons.size());
    anim.set_raw_model(m);
    update(0.0f, devices);
   
    if (has_anim()) {
        anim.play(0);

        size_t data_size = skin_data.size();
        skin_ssbo.reserve(data_size);
        skin_gpu_addresses.reserve(data_size);
        // todo - tripple buffer buffers
        for (size_t i = 0; i < data_size; i++) {
            if (skin_data[i].empty())
                continue;
            vk::buffer::handlers& vk_buffer = skin_ssbo.emplace_back();
            VkDeviceAddress& vk_address = skin_gpu_addresses.emplace_back();
            VkBufferUsageFlags flags_sk[2] = {VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};
            vk::buffer::create(vk_buffer, devices, flags_sk, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(glm::mat4) * skin_data[i].size(), skin_data[i].data());
            vk::buffer::get_gpu_address(vk_buffer, devices.dev);
            vk_address = vk_buffer.gpu_address;
        }
    }
}

void model::inst::update(float dt, vk::device::handlers devices)
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
        if (n.skin_id < 0 || n.skin_id > skin_data.size() || skin_data.empty()) continue;
        const auto& skelets = raw_model->info.skeletons[n.skin_id];
        auto& data = skin_data[n.skin_id];
        if (data.size() != skelets.bones.size())
            data.resize(skelets.bones.size());
        const glm::mat4 inv_mat = glm::inverse(global[i]);
        for (size_t j = 0; j < skelets.bones.size(); ++j) {
            data[j] = global[skelets.bones[j]] * skelets.inv_matrices[j];
            data[j] = inv_mat * data[j];
        }
    }

    int i = -1;
    for (std::vector<glm::mat4>& s : skin_data) {
        i++;
        if (s.empty() || skin_ssbo.empty() || skin_ssbo.size() < i)
            continue;
        vk::buffer::map_memory(skin_ssbo[i], devices.dev, s.size() * sizeof(glm::mat4), 0, s.data());
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

