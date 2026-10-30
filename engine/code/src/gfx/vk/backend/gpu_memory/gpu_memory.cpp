module;
#include "aai/gfx/vk/backend/vk_defines.h"
#include "glm/ext/matrix_transform.hpp"
#include "aai/gfx/vk/model/model_types.h"
#include <string.h>

module aai.gfx.vk.gpu_memory;
import aai.utils.mem;

void vk::gpu_memory::init(vk::device::handlers dev)
{
    init_descriptor_set(dev);
    init_buffers(dev);
}

const VkDeviceAddress vk::gpu_memory::get_buffer_address(const gpu_data_type& t, const uint32_t frame) const
{
   switch (t) {
       case gpu_data_type::CAMERA:
           return camera[frame].data.gpu_address;
       case gpu_data_type::INSTANCE:
           return instance[frame].data.gpu_address;
       default:
           return 0;
           break;
   }
   return 0;
}

void vk::gpu_memory::submit_camera(std::shared_ptr<const keeper::camera> cam, const std::uint32_t frame)
{
    camera[frame].raw_data.view = cam->get_view();
    camera[frame].raw_data.proj = cam->get_reverse_proj();
    camera[frame].raw_data.non_reverse_proj = cam->get_proj();
    camera[frame].submitted = true;
}

void vk::gpu_memory::submit_instance(const std::deque<rq::data>& rq, const std::uint32_t frame)
{
    if (!instance[frame].mapped_data)
        return;
    int ind = 0;
    float shift_x = 1.0f;
    for (const rq::data& data : rq) {
        instance_data d{};
        d.model = data.model_matrix;
        shift_x+= 2;
        utils::ASSERT(ind > instance[frame].buffer_size, "gpu_memory::submit_instance: buffer overflow");
        memcpy(&instance[frame].mapped_data[ind], &d, sizeof(instance_data));
        ind++;
    }
}

std::vector<VkDeviceAddress>& vk::gpu_memory::get_skin_gpu_address(const std::shared_ptr<model::skin_groups> group, vk::device::handlers devices)
{
    if (!group || group->skin_id <= 0)
        return dummy_skin_address;

    if (skin_ssbos.find(group->skin_id) == skin_ssbos.end()) {
        size_t data_size = group->d.size();
        skin_ssbos[group->skin_id].buffer.reserve(data_size);
        skin_ssbos[group->skin_id].gpu_addresses.reserve(data_size);
        // todo - tripple buffer buffers
        for (size_t i = 0; i < data_size; i++) {
            if (group->d[i].empty())
                continue;
            vk::buffer::handlers& vk_buffer = skin_ssbos[group->skin_id].buffer.emplace_back();
            VkDeviceAddress& vk_address = skin_ssbos[group->skin_id].gpu_addresses.emplace_back();
            VkBufferUsageFlags flags_sk[2] = {VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};
            vk::buffer::create(vk_buffer, devices, flags_sk, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(glm::mat4) * group->d[i].size(), group->d[i].data());
            vk::buffer::get_gpu_address(vk_buffer, devices.dev);
            vk_address = vk_buffer.gpu_address;
        }
        return skin_ssbos[group->skin_id].gpu_addresses;
    }
    else {
        int i = -1;
        for (const std::vector<glm::mat4>& s : group->d) {
            i++;
            if (s.empty() || skin_ssbos[group->skin_id].buffer.empty() || skin_ssbos[group->skin_id].buffer.size() < i)
                continue;
            vk::buffer::map_memory(skin_ssbos[group->skin_id].buffer[i],devices.dev, s.size() * sizeof(glm::mat4), 0, s.data());
        }
        return skin_ssbos[group->skin_id].gpu_addresses;
    }
}

void vk::gpu_memory::submit_debug(std::map<gfx::dbg::primitive_t, std::vector<gfx::dbg::point>>& debug_data, vk::device::handlers devices)
{
    if ( !debug_data[gfx::dbg::primitive_t::POINT].empty()) { 
        vk::buffer::map_memory(dbg_points.data, devices.dev, debug_data[gfx::dbg::primitive_t::POINT].size() * sizeof(gfx::dbg::point), 0, debug_data[gfx::dbg::primitive_t::POINT].data());
    }
    if ( !debug_data[gfx::dbg::primitive_t::LINE].empty()){
        vk::buffer::map_memory(dbg_line.data, devices.dev, debug_data[gfx::dbg::primitive_t::LINE].size() * sizeof(gfx::dbg::point), 0, debug_data[gfx::dbg::primitive_t::LINE].data());
    }
}

void vk::gpu_memory::update(vk::device::handlers dev, const std::uint32_t frame)
{
    if (camera[frame].submitted) {
        size_t buffer_size = sizeof(camera_data);
        vk::buffer::map_memory(camera[frame].data, dev.dev, buffer_size, 0, &camera[frame].raw_data);
    }

}

void vk::gpu_memory::update_texture(VkDevice dev, const std::string& name, VkImageView view, VkSampler sampler)
{
	texture_handle++;
    auto it = std::find_if(texture_bindings.begin(), texture_bindings.end(), [&, name](const auto& n) { return n.first == name; });
	if (it != texture_bindings.end()) {
        return ;
    }

    VkDescriptorImageInfo imageinfo{};
	imageinfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageinfo.imageView = view;
	imageinfo.sampler = sampler;

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write.dstBinding = 0;
	write.dstSet = bindless_texture_descriptor;
	write.descriptorCount = 1;
	write.pImageInfo = &imageinfo;
    
	write.dstArrayElement = texture_handle;

	vkUpdateDescriptorSets(dev, 1, &write, 0, nullptr);
    texture_bindings[name] = texture_handle;
}

VkDescriptorSetLayoutBinding descriptor_layout_binding(VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t binding)
{
	VkDescriptorSetLayoutBinding setbind = {};
	setbind.binding = binding;
	setbind.descriptorCount = 1000;
	setbind.descriptorType = type;
	setbind.pImmutableSamplers = nullptr;
	setbind.stageFlags = stageFlags;
	return setbind;
}
void vk::gpu_memory::init_descriptor_set(vk::device::handlers dev)
{
    {
    	VkDescriptorSetLayoutBinding bindings;
    	VkShaderStageFlags stageflags = VK_SHADER_STAGE_FRAGMENT_BIT;
    	VkDescriptorBindingFlags flags;
    	VkDescriptorType types =  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    // descriptor pool
    	VkDescriptorPoolSize sizes = { types, 1000 };
    
    	VkDescriptorPoolCreateInfo poolinfo{};
    	poolinfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    	poolinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    	poolinfo.maxSets = 5000;
    	poolinfo.poolSizeCount = 1;
    	poolinfo.pPoolSizes = &sizes;
    	vkCreateDescriptorPool(dev.dev, &poolinfo, nullptr, &texture_pool);
    // bindless layout
    	bindings = descriptor_layout_binding(types, stageflags, 0);
    	flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
    
    	VkDescriptorSetLayoutBindingFlagsCreateInfo bindingflags{};
    	bindingflags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    	bindingflags.pNext = nullptr;
    	bindingflags.pBindingFlags = &flags;
    	bindingflags.bindingCount = 1;
    
    	VkDescriptorSetLayoutCreateInfo createinfo{};
    	createinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    	createinfo.bindingCount = 1;
    	createinfo.pBindings = &bindings;
    	createinfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    	createinfo.pNext = &bindingflags;
    
    	vkCreateDescriptorSetLayout(dev.dev, &createinfo, nullptr, &bindless_texture_layout);
    
    // bindless descriptor
        VkDescriptorSetAllocateInfo allocinfo{};
        allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocinfo.pNext = nullptr;
        allocinfo.descriptorPool = texture_pool;
        allocinfo.pSetLayouts = &bindless_texture_layout;
        allocinfo.descriptorSetCount = 1;
        vkAllocateDescriptorSets(dev.dev, &allocinfo, &bindless_texture_descriptor);
    }
    utils::mem::get()->push(utils::mem::event::DELETE, utils::mem::type::VK, [=,this]() { vkDestroyDescriptorSetLayout(dev.dev, bindless_texture_layout, nullptr); });
    utils::mem::get()->push(utils::mem::event::DELETE, utils::mem::type::VK, [=,this]() { vkDestroyDescriptorPool(dev.dev, texture_pool, nullptr); });
}

void create_buffer(vk::device::handlers dev, vk::buffer::handlers& data, VkBufferUsageFlagBits usage, VkMemoryPropertyFlags properties, size_t buffer_size)
{
    vk::buffer::allocate(data, dev, buffer_size, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | usage, properties);
    vk::buffer::get_gpu_address(data, dev.dev);
    utils::mem::get()->push(utils::mem::event::DELETE, utils::mem::type::VK, [=]() { vkDestroyBuffer(dev.dev, data.buffer, nullptr); });
    utils::mem::get()->push(utils::mem::event::DELETE, utils::mem::type::VK, [=]() { vkFreeMemory(dev.dev, data.device_memory, nullptr); });
}

void vk::gpu_memory::init_buffers(vk::device::handlers dev)
{
    // todo - tripple buffer buffers
    size_t buffer_size = sizeof(camera_data);
    for (int i = 0; i < MAX_FRAMES; i++) {
        create_buffer(dev, camera[i].data, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer_size);

        // todo - instance size hardcoded
        buffer_size = sizeof(instance_data) * 1000;
        create_buffer(dev, instance[i].data, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer_size);
        instance[i].buffer_size = buffer_size;

        void* ptr_data;
        vkMapMemory(dev.dev , instance[i].data.device_memory, 0, buffer_size, 0, (void**)&ptr_data);
        instance[i].mapped_data = (instance_data*)ptr_data;
        utils::mem::get()->push(utils::mem::event::DELETE, utils::mem::type::VK, [=,this]() {vkUnmapMemory(dev.dev, instance[i].data.device_memory);});
    }
    
    buffer_size = sizeof(gfx::dbg::point) * 1000;
    create_buffer(dev, dbg_points.data, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer_size);
    buffer_size = sizeof(gfx::dbg::point) * 1000;
    create_buffer(dev, dbg_line.data, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer_size);
    gfx::dbg::draw::get()->set_vk_bind(gfx::dbg::primitive_t::LINE, dbg_line.data.gpu_address);
    gfx::dbg::draw::get()->set_vk_bind(gfx::dbg::primitive_t::POINT, dbg_points.data.gpu_address);
}


