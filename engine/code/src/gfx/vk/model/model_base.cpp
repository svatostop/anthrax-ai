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


