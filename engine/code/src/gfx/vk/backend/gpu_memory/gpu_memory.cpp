#include "aai/gfx/vk/backend/vk_defines.h" 
import aai.gfx.vk.gpu_memory;

void vk::gpu_memory::init(vk::device::handlers dev)
{
    init_descriptor_set(dev);
    init_buffers(dev);
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

}
void vk::gpu_memory::init_buffers(vk::device::handlers dev)
{
    size_t buffer_size = sizeof(camera_data);
    vk::buffer::allocate(camera, dev, buffer_size, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vk::buffer::get_gpu_address(camera, dev.dev);
}


