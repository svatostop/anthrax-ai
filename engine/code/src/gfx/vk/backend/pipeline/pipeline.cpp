#include "aai/gfx/vk/backend/vk_defines.h"
#include <shaderc/shaderc.h>

import aai.gfx.vk.pipeline;
import aai.gfx.vk.pipeline.helper;
import aai.gfx.vk.loader.shader;
import aai.utils;
import glm;
import std;
VkPipelineVertexInputStateCreateInfo vertex_input_create_info(bool no_vertex)
{
    VkPipelineVertexInputStateCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	info.pNext = nullptr;
    if (no_vertex) {
        info.pVertexAttributeDescriptions = nullptr;
	    info.vertexAttributeDescriptionCount = 0;

    	info.pVertexBindingDescriptions = nullptr;
        info.vertexBindingDescriptionCount =0;
    	return info;
    }   
    vk::pipeline::vertex_desc desc;

	VkVertexInputBindingDescription mainBinding = {};
	mainBinding.binding = 0;
	mainBinding.stride = sizeof(vk::pipeline::vertex);
    mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	desc.bindings.push_back(mainBinding);


    VkVertexInputAttributeDescription positionAttribute = {};
    positionAttribute.binding = 0;
	positionAttribute.location = 0;
    positionAttribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    positionAttribute.offset = offsetof(vk::pipeline::vertex, position);
    
    VkVertexInputAttributeDescription normalAttribute = {};
    normalAttribute.binding = 0;
    normalAttribute.location = 1;
    normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    normalAttribute.offset = offsetof(vk::pipeline::vertex, normal);
    
    VkVertexInputAttributeDescription colorAttribute = {};
    colorAttribute.binding = 0;
    colorAttribute.location = 2;
    colorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    colorAttribute.offset = offsetof(vk::pipeline::vertex, color);
    
    VkVertexInputAttributeDescription uvattr = {};
    uvattr.binding = 0;
    uvattr.location = 3;
    uvattr.format = VK_FORMAT_R32G32_SFLOAT;
    uvattr.offset = offsetof(vk::pipeline::vertex, uv);
    VkVertexInputAttributeDescription weightattr = {};
    weightattr.binding = 0;
    weightattr.location = 4;
    weightattr.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    weightattr.offset = offsetof(vk::pipeline::vertex, weights );
    VkVertexInputAttributeDescription boneattr = {};
    weightattr.binding = 0;
    weightattr.location = 5;
    weightattr.format = VK_FORMAT_R32G32B32A32_UINT;
    weightattr.offset = offsetof(vk::pipeline::vertex, boneID);

    desc.attributes.push_back(positionAttribute);
    desc.attributes.push_back(normalAttribute);
    desc.attributes.push_back(colorAttribute);
    desc.attributes.push_back(uvattr);
    desc.attributes.push_back(weightattr);
    desc.attributes.push_back(boneattr);

	info.pVertexAttributeDescriptions = desc.attributes.data();
	info.vertexAttributeDescriptionCount = desc.attributes.size();

	info.pVertexBindingDescriptions = desc.bindings.data();
	info.vertexBindingDescriptionCount = desc.bindings.size();
	return info;
}
VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info()
{
    VkPipelineInputAssemblyStateCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	info.pNext = nullptr;

	info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	info.primitiveRestartEnable = VK_FALSE;
	return info;
}
VkViewport convert_and_apply_viewport(const glm::vec4& viewport)
{
    VkViewport v = {};
    v.x = viewport.z;
	v.y = viewport.w;
	v.width = viewport.x;
	v.height = viewport.y;
	v.minDepth = 0.0f;
	v.maxDepth = 1.0f;
    return v;
}
VkRect2D convert_and_apply_scissor(const glm::vec4& scissor)
{
    VkRect2D s = {};
    s.offset.x = static_cast<uint32_t>(scissor.z);
    s.offset.y = static_cast<uint32_t>(scissor.w);
	s.extent.width = static_cast<uint32_t>(scissor.x);
    s.extent.height = static_cast<uint32_t>(scissor.y );
    return s;
}
VkPipelineRasterizationStateCreateInfo convert_and_apply_rasterizer(const mat::rasterizer_helper raster_info)
{
    VkPipelineRasterizationStateCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	info.pNext = nullptr;

	info.depthClampEnable = VK_FALSE;
	info.rasterizerDiscardEnable = VK_FALSE;

	info.polygonMode = vk::convert::polygon(raster_info.polygon);
	info.lineWidth = 1.0f;
	info.cullMode = vk::convert::cull(raster_info.cull);
	info.frontFace = vk::convert::face(raster_info.face);
	info.depthBiasEnable = VK_FALSE;
	info.depthBiasConstantFactor = 0.0f;
	info.depthBiasClamp = 0.0f;
	info.depthBiasSlopeFactor = 0.0f;

	return info;
}
VkPipelineMultisampleStateCreateInfo multisampling_create_info(const bool multisampling)
{
    VkPipelineMultisampleStateCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	info.pNext = nullptr;

	info.sampleShadingEnable = VK_FALSE;
	info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	info.minSampleShading = 1.0f;
	info.pSampleMask = nullptr;
	info.alphaToCoverageEnable = VK_FALSE;
	info.alphaToOneEnable = VK_FALSE;
	return info;
}

VkPipelineColorBlendAttachmentState convert_and_apply_color_blend(const mat::color_blend_helper cb) {

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = cb.alpha_blend;
	colorBlendAttachment.srcColorBlendFactor = vk::convert::blend_factor(cb.src_color_val);
	colorBlendAttachment.dstColorBlendFactor = vk::convert::blend_factor(cb.dst_color_val);
	colorBlendAttachment.colorBlendOp = vk::convert::blend_op(cb.c_op);
	colorBlendAttachment.srcAlphaBlendFactor = vk::convert::blend_factor(cb.src_alpha_val);;
	colorBlendAttachment.dstAlphaBlendFactor = vk::convert::blend_factor(cb.dst_alpha_val);
	colorBlendAttachment.alphaBlendOp = vk::convert::blend_op(cb.a_op);
	return colorBlendAttachment;
}

VkPipelineDepthStencilStateCreateInfo convert_and_apply_depth_stencil(const mat::depth_helper d)
{
    VkPipelineDepthStencilStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    info.pNext = nullptr;

    info.depthTestEnable = d.depth_test;
    info.depthWriteEnable = d.depth_write;
    info.depthCompareOp = d.depth_test ? vk::convert::depth_cmp(d.d_op) : VK_COMPARE_OP_ALWAYS;
    info.depthBoundsTestEnable = VK_FALSE;
    info.minDepthBounds = 0.0f; 
    info.maxDepthBounds = 1.0f; 
    info.stencilTestEnable = VK_FALSE;

    return info;
}

VkPipelineLayoutCreateInfo vk::pipeline::pipeline_layout_create_info()
{
	VkPipelineLayoutCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	info.pNext = nullptr;

    VkPushConstantRange push_constant;
	push_constant.offset = 0;
	push_constant.size = sizeof(vk::pipeline::push_range);
	push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	info.pPushConstantRanges = &push_constant;
	info.pushConstantRangeCount = 1;

    VkDescriptorSetLayout setLayouts[] = {  bindless_texture_layout };
	info.setLayoutCount = 1;
	info.pSetLayouts = setLayouts;

	return info;
}
VkShaderModule set_shader(VkDevice dev, const std::string& buffer)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.codeSize = buffer.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

	VkShaderModule shadermodule;
    utils::VK_ASSERT(vkCreateShaderModule(dev, &createInfo, nullptr, &shadermodule), "failed to create shader module!");
	return shadermodule;
}
VkPipelineShaderStageCreateInfo shader_create_info(VkShaderStageFlagBits stage, VkShaderModule shadermodule)
{
	VkPipelineShaderStageCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	info.pNext = nullptr;

	info.stage = stage;
	info.module = shadermodule;
	info.pName = "main";
	return info;
}

void build_shader(VkDevice dev, mat::shader_module module, std::vector<VkPipelineShaderStageCreateInfo>& shader_stages)
{
    shaderc_shader_kind type = vk::convert::shader_type_sc(module.t);
    std::string shaderbuf;
    loader::shader::compile(module.path, type, shaderbuf);
    shader_stages.push_back(shader_create_info(vk::convert::shader_type_vk(module.t), set_shader(dev, shaderbuf)));
}

VkPipelineViewportStateCreateInfo get_viewport_state(VkViewport viewport, VkRect2D scissor)
{
	VkPipelineViewportStateCreateInfo viewportstate = {};
	viewportstate.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportstate.pNext = nullptr;

	viewportstate.viewportCount = 1;
	viewportstate.pViewports = &viewport;
	viewportstate.scissorCount = 1;
	viewportstate.pScissors = &scissor;

    return viewportstate;
}
VkPipelineColorBlendStateCreateInfo get_color_blend_state(VkPipelineColorBlendAttachmentState blend, const rt::attachment_ref::info& attachments)
{
    std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates;
    VkPipelineColorBlendStateCreateInfo colorblending = {};
	colorblending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorblending.pNext = nullptr;
	colorblending.logicOpEnable = VK_FALSE;
	colorblending.logicOp = VK_LOGIC_OP_COPY;
	colorblending.attachmentCount = attachments.color_count;
    for (uint32_t i = 0; i  < attachments.color_count; i++) {
        blendAttachmentStates.reserve(colorblending.attachmentCount);
        for (int i = 0; i < colorblending.attachmentCount; i++) {
            blendAttachmentStates.push_back(blend);
        }
        colorblending.pAttachments = blendAttachmentStates.data();
    }
    return colorblending;
}

VkPipelineRenderingCreateInfoKHR get_rendering_info(const rt::attachment_ref::info& attachments)
{
    VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo{};
    pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR; 
    pipelineRenderingCreateInfo.colorAttachmentCount = attachments.color_count;
    std::vector<VkFormat> color_formats;
    for (auto& att : attachments.color_types) {
        color_formats.push_back(att.format); 
    }
    if (attachments.color_count > 0) {
        pipelineRenderingCreateInfo.pColorAttachmentFormats = color_formats.data();
    }
    if (attachments.depth_count > 0) {
       pipelineRenderingCreateInfo.depthAttachmentFormat = attachments.depth_types.format;
    }
    return pipelineRenderingCreateInfo;
}

void vk::pipeline::create_material(VkDevice dev, mat::materials& m, const rt::attachment_ref::info& attachments)
{
    VkPipelineVertexInputStateCreateInfo 	vertex_input_info = vertex_input_create_info(m.get_info().vertex_attributes);
    VkPipelineInputAssemblyStateCreateInfo 	input_assembly = input_assembly_create_info();
    VkViewport 								viewport = convert_and_apply_viewport(m.get_info().viewport);
    VkRect2D 								scissor = convert_and_apply_scissor(m.get_info().scissor);
    VkPipelineRasterizationStateCreateInfo 	rasterizer = convert_and_apply_rasterizer(m.get_info().rasterizer);
    VkPipelineColorBlendAttachmentState 	color_blend = convert_and_apply_color_blend(m.get_info().color_blend);
    VkPipelineMultisampleStateCreateInfo 	multisampling = multisampling_create_info(m.get_info().multisampling);
    VkPipelineDepthStencilStateCreateInfo 	depth_stencil = convert_and_apply_depth_stencil(m.get_info().depth_stencil);

	VkPipelineLayoutCreateInfo pipelinelayoutinfo = pipeline_layout_create_info();

    utils::VK_ASSERT(vkCreatePipelineLayout(dev, &pipelinelayoutinfo, nullptr, &pipe_layout), "failed to create pipeline layout!");
    // auto future = asset_mng.load_async(path, [&](const std::string&) {  
    //    load_shader(shader); 
    //     return nullptr;
    // });
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    for (auto& shader : m.get_info().shaders)
        build_shader(dev, shader, shader_stages);

    VkPipelineViewportStateCreateInfo viewport_state = get_viewport_state(viewport, scissor);
    VkPipelineColorBlendStateCreateInfo color_blend_state = get_color_blend_state(color_blend, attachments);
	VkPipelineRenderingCreateInfoKHR rendering_info = get_rendering_info(attachments); 

    VkGraphicsPipelineCreateInfo pipelineinfo = {};
	pipelineinfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineinfo.pNext = &rendering_info;
	pipelineinfo.stageCount = shader_stages.size();
	pipelineinfo.pStages = shader_stages.data();
	pipelineinfo.pVertexInputState = &vertex_input_info;
	pipelineinfo.pInputAssemblyState = &input_assembly;
	pipelineinfo.pViewportState = &viewport_state;
	pipelineinfo.pRasterizationState = &rasterizer;
	pipelineinfo.pMultisampleState = &multisampling;
	pipelineinfo.pColorBlendState = &color_blend_state;
	pipelineinfo.pDepthStencilState = &depth_stencil;
	pipelineinfo.layout = pipe_layout;
	pipelineinfo.renderPass = nullptr;// renderer.getrenderpass();
	pipelineinfo.subpass = 0;

    utils::VK_ASSERT(vkCreateGraphicsPipelines(dev, VK_NULL_HANDLE, 1, &pipelineinfo, nullptr, &pipe), "failed to create write pipeline\n");

    m.set_data(m.get_info().name, pipe, pipe_layout);
}

