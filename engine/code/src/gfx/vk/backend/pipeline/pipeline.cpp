module;
#include "aai/gfx/vk/model/model_types.h"
#include "aai/gfx/vk/backend/vk_defines.h"
#include <shaderc/shaderc.h>

module aai.gfx.vk.pipeline;
import aai.gfx.vk.pipeline.helper;
import aai.gfx.vk.loader.shader;
import aai.utils;
import glm;
import std;
VkPipelineVertexInputStateCreateInfo vk::pipeline::vertex_input_create_info(bool no_vertex)
{
    VkPipelineVertexInputStateCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	info.pNext = nullptr;
    info.flags = 0;
    if (!no_vertex) {
        info.pVertexAttributeDescriptions = nullptr;
	    info.vertexAttributeDescriptionCount = 0;

    	info.pVertexBindingDescriptions = nullptr;
        info.vertexBindingDescriptionCount =0;
        return info;
    }   
    
    vert_desc_info.bindings.clear();
    vert_desc_info.attributes.clear();

	VkVertexInputBindingDescription mainBinding = {};
	mainBinding.binding = 0;
	mainBinding.stride = sizeof(model::types::vertex);
    mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	vert_desc_info.bindings.push_back(mainBinding);

    VkVertexInputAttributeDescription positionAttribute = {};
    positionAttribute.binding = 0;
	positionAttribute.location = 0;
    positionAttribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    positionAttribute.offset = offsetof(model::types::vertex, position);
    
    VkVertexInputAttributeDescription normalAttribute = {};
    normalAttribute.binding = 0;
    normalAttribute.location = 1;
    normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    normalAttribute.offset = offsetof(model::types::vertex, normal);
    
    VkVertexInputAttributeDescription colorAttribute = {};
    colorAttribute.binding = 0;
    colorAttribute.location = 2;
    colorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    colorAttribute.offset = offsetof(model::types::vertex, color);
    
    VkVertexInputAttributeDescription uvattr = {};
    uvattr.binding = 0;
    uvattr.location = 3;
    uvattr.format = VK_FORMAT_R32G32_SFLOAT;
    uvattr.offset = offsetof(model::types::vertex, uv);
    VkVertexInputAttributeDescription weightattr = {};
    weightattr.binding = 0;
    weightattr.location = 4;
    weightattr.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    weightattr.offset = offsetof(model::types::vertex, weights );
    VkVertexInputAttributeDescription boneattr = {};
    boneattr.binding = 0;
    boneattr.location = 5;
    boneattr.format = VK_FORMAT_R32G32B32A32_UINT;
    boneattr.offset = offsetof(model::types::vertex, boneID);

    vert_desc_info.attributes.push_back(positionAttribute);
    vert_desc_info.attributes.push_back(normalAttribute);
    vert_desc_info.attributes.push_back(colorAttribute);
    vert_desc_info.attributes.push_back(uvattr);
    vert_desc_info.attributes.push_back(weightattr);
    vert_desc_info.attributes.push_back(boneattr);

    info.pVertexAttributeDescriptions = vert_desc_info.attributes.data();
    info.vertexAttributeDescriptionCount = vert_desc_info.attributes.size();

    info.pVertexBindingDescriptions = vert_desc_info.bindings.data();
    info.vertexBindingDescriptionCount = vert_desc_info.bindings.size();
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
void convert_and_apply_viewport(VkViewport& v, const glm::vec4& viewport)
{
    v.x = viewport.z;
	v.y = viewport.w;
	v.width = viewport.x;
	v.height = viewport.y;
	v.minDepth = 0.0f;
	v.maxDepth = 1.0f;
}
void convert_and_apply_scissor(VkRect2D& s, const glm::vec4& scissor)
{
    s.offset.x = static_cast<uint32_t>(scissor.z);
    s.offset.y = static_cast<uint32_t>(scissor.w);
	s.extent.width = static_cast<uint32_t>(scissor.x);
    s.extent.height = static_cast<uint32_t>(scissor.y );
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

void get_color_blend_state(VkPipelineColorBlendStateCreateInfo& color_blend_state, VkPipelineColorBlendAttachmentState blend, const rt::base::ref& attachments)
{
	color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_state.pNext = nullptr;
    color_blend_state.flags = 0;
	color_blend_state.logicOpEnable = VK_FALSE;
	color_blend_state.logicOp = VK_LOGIC_OP_COPY;
}

VkPipelineRenderingCreateInfoKHR get_rendering_info(const rt::base::ref& attachments)
{
    VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo{};
    pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR; 
    pipelineRenderingCreateInfo.colorAttachmentCount = attachments.color_count;
    return pipelineRenderingCreateInfo;
}

void vk::pipeline::create_material(VkDevice dev, mat::materials& m)
{
    vertex_input_info = vertex_input_create_info(m.get_info().vertex_attributes);
    VkPipelineInputAssemblyStateCreateInfo 	input_assembly = input_assembly_create_info();
    convert_and_apply_viewport(viewport, m.get_info().viewport);
    convert_and_apply_scissor(scissor, m.get_info().scissor);
    VkPipelineRasterizationStateCreateInfo 	rasterizer = convert_and_apply_rasterizer(m.get_info().rasterizer);
    VkPipelineColorBlendAttachmentState 	color_blend = convert_and_apply_color_blend(m.get_info().color_blend);
    VkPipelineMultisampleStateCreateInfo 	multisampling = multisampling_create_info(m.get_info().multisampling);
    VkPipelineDepthStencilStateCreateInfo 	depth_stencil = convert_and_apply_depth_stencil(m.get_info().depth_stencil);

    VkPipelineLayoutCreateInfo pipelinelayoutinfo{}; 
    pipelinelayoutinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelinelayoutinfo.pNext = nullptr;
    VkPushConstantRange push_constant;
	push_constant.offset = 0;
	push_constant.size = sizeof(vk::pipeline::push_range);
	push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pipelinelayoutinfo.pPushConstantRanges = &push_constant;
	pipelinelayoutinfo.pushConstantRangeCount = 1;
    VkDescriptorSetLayout setLayouts[] = {  bindless_texture_layout };
    if (m.get_info().bind_texture) {
        pipelinelayoutinfo.setLayoutCount = 1;
        pipelinelayoutinfo.pSetLayouts = setLayouts;
    }
    else {
        pipelinelayoutinfo.setLayoutCount = 0;
        pipelinelayoutinfo.pSetLayouts = nullptr;
    }

    utils::VK_ASSERT(vkCreatePipelineLayout(dev, &pipelinelayoutinfo, nullptr, &pipe_layout), "failed to create pipeline layout!");
    // auto future = asset_mng.load_async(path, [&](const std::string&) {  
    //    load_shader(shader); 
    //     return nullptr;
    // });
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    for (auto& shader : m.get_info().shaders)
        build_shader(dev, shader, shader_stages);

    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.pNext = nullptr;
    viewport_state.flags = 0;
    
    VkPipelineDynamicStateCreateInfo dynamic_states{};
    dynamic_states.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_states.flags = 0;
    dynamic_states.dynamicStateCount = 0;
    dynamic_states.pDynamicStates = nullptr;

    if (m.get_info().dynamic_viewport) {
        viewport_state.viewportCount = 1;
        viewport_state.pViewports = nullptr;
        viewport_state.scissorCount = 1;
        viewport_state.pScissors = nullptr;
        dynamic_states.dynamicStateCount = 2;
        VkDynamicState states[2];
        states[0] = VK_DYNAMIC_STATE_VIEWPORT;
        states[1] = VK_DYNAMIC_STATE_SCISSOR;
        dynamic_states.pDynamicStates = states;
    }
    else {
        viewport_state.viewportCount = 1;
        viewport_state.pViewports = &viewport;
        viewport_state.scissorCount = 1;
        viewport_state.pScissors = &scissor;
    }
    rt::base::ref attachments = m.get_info().rt_ref;
    get_color_blend_state(color_blend_state, color_blend, attachments);
    std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates;
    color_blend_state.attachmentCount = attachments.color_count;
    for (uint32_t i = 0; i  < attachments.color_count; i++) {
        blendAttachmentStates.reserve(color_blend_state.attachmentCount);
        for (int i = 0; i < color_blend_state.attachmentCount; i++) {
            blendAttachmentStates.push_back(color_blend);
        }
        color_blend_state.pAttachments = blendAttachmentStates.data();
    }

	VkPipelineRenderingCreateInfoKHR rendering_info = get_rendering_info(attachments); 
    std::vector<VkFormat> color_formats;
    for (auto& att : attachments.color_types) {
        color_formats.push_back(att.format); 
    }
    if (attachments.color_count > 0) {
        rendering_info.pColorAttachmentFormats = color_formats.data();
    }
    if (attachments.depth_count > 0) {
        rendering_info.depthAttachmentFormat = attachments.depth_types.format;
    }

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
    pipelineinfo.pDynamicState = &dynamic_states ;
	pipelineinfo.renderPass = nullptr;// renderer.getrenderpass();
	pipelineinfo.subpass = 0;

    utils::VK_ASSERT(vkCreateGraphicsPipelines(dev, VK_NULL_HANDLE, 1, &pipelineinfo, nullptr, &pipe), "failed to create write pipeline\n");

    for (VkPipelineShaderStageCreateInfo& shader_stage : shader_stages) {
        vkDestroyShaderModule(dev, shader_stage.module, nullptr);
    }

    m.set_data(m.get_info().name, pipe, pipe_layout, m.get_info().rt_ref, m.get_info().dynamic_viewport);
}

