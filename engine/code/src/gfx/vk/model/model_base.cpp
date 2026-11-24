module;
#include "aai/gfx/vk/loaders/model_loader.h"

module aai.gfx.vk.model;
import aai.utils;
import aai.utils.timer;

void model::base::load(const std::string& p, vk::device::handlers devices)
{
    loader::gltf::load(p, info);
   
    path = p;
    utils::ASSERT(info.vertex_buffer.empty(), "loader::gltf::load(): returned emprty vertex buffer");
    utils::ASSERT(info.index_buffer.empty(), "loader::gltf::load(): returned emprty index buffer");
    VkBufferUsageFlags flags[2] = {VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT};
    vk::buffer::create(data.vertices, devices, flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(info.vertex_buffer[0]) * info.vertex_buffer.size(), info.vertex_buffer.data());
	VkBufferUsageFlags flags2[2] = {VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT};
    vk::buffer::create(data.indices, devices, flags2, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(info.index_buffer[0]) * info.index_buffer.size(), info.index_buffer.data());
    
}

void model::base::load_simple(const std::string& p, const std::vector<types::vertex>& vertex_buffer, const std::vector<uint16_t>& index_buffer, vk::device::handlers devices)
{
   path = p;

   info.vertex_buffer = vertex_buffer;
   if (!index_buffer.empty()) {
       info.index_buffer = index_buffer;
   }
   else {
       info.index_buffer = default_indices;
   }

   VkBufferUsageFlags flags[2] = {VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT};
    vk::buffer::create(data.vertices, devices, flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(info.vertex_buffer[0]) * info.vertex_buffer.size(), info.vertex_buffer.data());
	VkBufferUsageFlags flags2[2] = {VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT};
    vk::buffer::create(data.indices, devices, flags2, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(info.index_buffer[0]) * info.index_buffer.size(), info.index_buffer.data());

}
