#include <vulkan/vulkan_core.h>
import aai.gfx.vk.pipeline;
import aai.gfx.vk.pipeline.helper;
import glm;

VkPipelineVertexInputStateCreateInfo vertex_input_create_info()
{
    VkPipelineVertexInputStateCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	info.pNext = nullptr;
    
    // todo
	// info.pVertexAttributeDescriptions = VertexDescription.Attributes.data();
	// info.vertexAttributeDescriptionCount = VertexDescription.Attributes.size();

	// info.pVertexBindingDescriptions = VertexDescription.Bindings.data();
	// info.vertexBindingDescriptionCount = VertexDescription.Bindings.size();
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

void vk::pipeline::create_material(mat::materials& m)
{
    VkPipelineVertexInputStateCreateInfo 	vertex_input_info = vertex_input_create_info();
    VkPipelineInputAssemblyStateCreateInfo 	input_assembly = input_assembly_create_info();
    VkViewport 								viewport = convert_and_apply_viewport(m.get_info().viewport);
    VkRect2D 								scissor = convert_and_apply_scissor(m.get_info().scissor);
    VkPipelineRasterizationStateCreateInfo 	rasterizer = convert_and_apply_rasterizer(m.get_info().rasterizer);
    VkPipelineColorBlendAttachmentState 	color_blend = convert_and_apply_color_blend(m.get_info().color_blend);
    VkPipelineMultisampleStateCreateInfo 	multisampling = multisampling_create_info(m.get_info().multisampling);
    VkPipelineDepthStencilStateCreateInfo 	depth_stencil = convert_and_apply_depth_stencil(m.get_info().depth_stencil);
}

