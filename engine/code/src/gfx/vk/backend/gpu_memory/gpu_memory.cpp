module;
#include "aai/gfx/vk/backend/vk_defines.h"
#include "glm/ext/matrix_transform.hpp"
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
        d.model = glm::translate(data.model_matrix, glm::vec3(0.0 + shift_x, -1.0 + shift_x, 1.0));
        shift_x+= 2;
        utils::ASSERT(ind > instance[frame].buffer_size, "gpu_memory::submit_instance: buffer overflow");
        memcpy(&instance[frame].mapped_data[ind], &d, sizeof(instance_data));
        ind++;
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
void vk::gpu_memory::init_buffers(vk::device::handlers dev)
{
    // todo - tripple buffer buffers
    size_t buffer_size = sizeof(camera_data);
    for (int i = 0; i < MAX_FRAMES; i++) {
        vk::buffer::allocate(camera[i].data, dev, buffer_size, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vk::buffer::get_gpu_address(camera[i].data, dev.dev);
        utils::mem::get()->push(utils::mem::event::DELETE, utils::mem::type::VK, [=,this]() { vkDestroyBuffer(dev.dev, camera[i].data.buffer, nullptr); });
        utils::mem::get()->push(utils::mem::event::DELETE, utils::mem::type::VK, [=,this]() { vkFreeMemory(dev.dev, camera[i].data.device_memory, nullptr); });

        // todo - instance size hardcoded
        buffer_size = sizeof(instance_data) * 1000;
        vk::buffer::allocate(instance[i].data, dev, buffer_size, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vk::buffer::get_gpu_address(instance[i].data, dev.dev);
        instance[i].buffer_size = buffer_size;

        void* ptr_data;
        vkMapMemory(dev.dev , instance[i].data.device_memory, 0, buffer_size, 0, (void**)&ptr_data);
        instance[i].mapped_data = (instance_data*)ptr_data;
        utils::mem::get()->push(utils::mem::event::DELETE, utils::mem::type::VK, [=,this]() { vkDestroyBuffer(dev.dev, instance[i].data.buffer, nullptr); });
        utils::mem::get()->push(utils::mem::event::DELETE, utils::mem::type::VK, [=,this]() { vkFreeMemory(dev.dev, instance[i].data.device_memory, nullptr); });
        utils::mem::get()->push(utils::mem::event::DELETE, utils::mem::type::VK, [=,this]() {vkUnmapMemory(dev.dev, instance[i].data.device_memory);});
    }
}


