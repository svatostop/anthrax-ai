module;
#include "aai/gfx/vk/backend/loaders/model_loader.h"

module aai.gfx.vk.model;
import aai.utils;

void model::base::load(const std::string& path, vk::device::handlers devices)
{
    std::vector<types::vertex> verts;
    std::vector<uint16_t> inds;
    loader::gltf::load(path, nodes, inds, verts, skins, animations);
    
    utils::ASSERT(verts.empty(), "loader::gltf::load(): returned emprty vertex buffer");
    utils::ASSERT(inds.empty(), "loader::gltf::load(): returned emprty index buffer");
    VkBufferUsageFlags flags[2] = {VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT};
    vk::buffer::create(data.vertices, devices, flags, sizeof(verts[0]) * verts.size(), verts.data());
	VkBufferUsageFlags flags2[2] = {VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT};
    vk::buffer::create(data.indices, devices, flags2, sizeof(inds[0]) * inds.size(), inds.data());
}
