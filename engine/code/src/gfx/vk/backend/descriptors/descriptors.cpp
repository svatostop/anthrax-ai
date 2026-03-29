#include "aai/gfx/vk/backend/vk_defines.h" 
import aai.gfx.vk.descriptors;

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

void vk::descriptors::init(VkDevice dev)
{
    {
    	VkDescriptorSetLayoutBinding bindings[MAX_BINDING];
    	VkShaderStageFlags stageflags[MAX_BINDING] = {
    		VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    		VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    		VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_VERTEX_BIT,
    	};
    	VkDescriptorBindingFlags flags[MAX_BINDING];
    	VkDescriptorType types[MAX_BINDING] = {
    		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    	};
    
    // descriptor pool
    
    	VkDescriptorPoolSize sizes[MAX_BINDING] = {
    		{ types[0], 1000 }, { types[1], 1000 }, { types[3], 1000 }
    	};
    
    	VkDescriptorPoolCreateInfo poolinfo{};
    	poolinfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    	poolinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    	poolinfo.maxSets = 5000;
    	poolinfo.poolSizeCount = MAX_BINDING;
    	poolinfo.pPoolSizes = sizes;
        for (int i = 0; i < MAX_FRAMES; i++) {
    	    vkCreateDescriptorPool(dev, &poolinfo, nullptr, &buffer_pool[i]);
        }
    // bindless layout
    
    	for (int i = 0; i < MAX_BINDING; i++) {
    		bindings[i] = descriptor_layout_binding(types[i], stageflags[i], i);
    		flags[i] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
    	}
    
    	VkDescriptorSetLayoutBindingFlagsCreateInfo bindingflags{};
    	bindingflags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    	bindingflags.pNext = nullptr;
    	bindingflags.pBindingFlags = flags;
    	bindingflags.bindingCount = MAX_BINDING;
    
    	VkDescriptorSetLayoutCreateInfo createinfo{};
    	createinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    	createinfo.bindingCount = MAX_BINDING;
    	createinfo.pBindings = bindings;
    	createinfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    	createinfo.pNext = &bindingflags;
    
    	vkCreateDescriptorSetLayout(dev, &createinfo, nullptr, &bindless_buffer_layout);
    
    // bindless descriptor
        for (int i = 0; i < MAX_FRAMES; i++) {
        	VkDescriptorSetAllocateInfo allocinfo{};
        	allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        	allocinfo.pNext = nullptr;
        	allocinfo.descriptorPool = buffer_pool[i];
        	allocinfo.pSetLayouts = &bindless_buffer_layout;
        	allocinfo.descriptorSetCount = 1;
        	vkAllocateDescriptorSets(dev, &allocinfo, &bindless_buffer_descriptor[i]);
        }
    }
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
    	vkCreateDescriptorPool(dev, &poolinfo, nullptr, &texture_pool);
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
    
    	vkCreateDescriptorSetLayout(dev, &createinfo, nullptr, &bindless_texture_layout);
    
    // bindless descriptor
        VkDescriptorSetAllocateInfo allocinfo{};
        allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocinfo.pNext = nullptr;
        allocinfo.descriptorPool = texture_pool;
        allocinfo.pSetLayouts = &bindless_texture_layout;
        allocinfo.descriptorSetCount = 1;
        vkAllocateDescriptorSets(dev, &allocinfo, &bindless_texture_descriptor);
    }

}


