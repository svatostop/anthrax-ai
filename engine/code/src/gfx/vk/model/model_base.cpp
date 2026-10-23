module;
#include "aai/gfx/vk/loaders/model_loader.h"

module aai.gfx.vk.model;
import aai.utils;
import aai.utils.timer;

void model::base::load(const std::string& path, vk::device::handlers devices)
{
    loader::gltf::load(path, info);
    
    utils::ASSERT(info.vertex_buffer.empty(), "loader::gltf::load(): returned emprty vertex buffer");
    utils::ASSERT(info.index_buffer.empty(), "loader::gltf::load(): returned emprty index buffer");
    VkBufferUsageFlags flags[2] = {VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT};
    vk::buffer::create(data.vertices, devices, flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(info.vertex_buffer[0]) * info.vertex_buffer.size(), info.vertex_buffer.data());
	VkBufferUsageFlags flags2[2] = {VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT};
    vk::buffer::create(data.indices, devices, flags2, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(info.index_buffer[0]) * info.index_buffer.size(), info.index_buffer.data());
    
}

// void model::base::update_animation(float delta_ms, const vk::device::handlers& devices)
// {
//     if (animations.empty())
//         return;
//     if (active_anim > animations.size() - 1)
//         return;
//     float delta = delta_ms / 1000.0f;
//     model::animation::base& anim = animations[active_anim];
//     anim.current_time += delta;
//     if (anim.current_time > anim.end)
//         anim.current_time -= anim.end;
//     for (model::animation::channel& c : anim.channels) {
//         model::animation::sampler& sample = anim.samplers[c.sampler_index];
//         for (int i = 0; i < sample.inputs.size() - 1; i++) {
//             if (sample.interpolation != "LINEAR")
//                 continue;
//             if (anim.current_time >= sample.inputs[i] && anim.current_time <= sample.inputs[i + 1]) {
//                 float val = (anim.current_time - sample.inputs[i]) / (sample.inputs[i + 1] - sample.inputs[i]);
//                 if (c.path == "translation")
//                     c.n->translation = glm::mix(sample.outputs[i], sample.outputs[i + 1], val);
//                 if (c.path == "rotation") {
//                     glm::quat q1;
//                     q1.x = sample.outputs[i].x;
//                     q1.y = sample.outputs[i].y;
//                     q1.z = sample.outputs[i].z;
//                     q1.w = sample.outputs[i].w;
//                     glm::quat q2;
//                     q2.x = sample.outputs[i + 1].x;
//                     q2.y = sample.outputs[i + 1].y;
//                     q2.z = sample.outputs[i + 1].z;
//                     q2.w = sample.outputs[i + 1].w;
//                     c.n->rotation = glm::normalize(glm::slerp(q1,q2,val));
//                 }
//                 if (c.path == "scale") {
//                     c.n->scale = glm::mix(sample.outputs[i], sample.outputs[i + 1], val);
//                 }
//             }
//         }
//     }
//
//     for (auto& n : nodes) {
//         loader::gltf::update_bones(n, skins);
//     }
//     int i = -1;
//     for (model::types::skin& s : skins) {
//         i++;
//         if (s.fin_bone_transforms.empty())
//             continue;
//         vk::buffer::map_memory(data.skin[i], devices.dev, s.fin_bone_transforms.size() * sizeof(glm::mat4), 0, s.fin_bone_transforms.data());
//     }
// }
