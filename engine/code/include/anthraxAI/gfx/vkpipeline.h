#pragma once

#include "anthraxAI/gfx/renderhelpers.h"
#include "anthraxAI/utils/defines.h"
#include "anthraxAI/utils/debug.h"
#include "anthraxAI/gfx/vkdefines.h"
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <shaderc/shaderc.hpp>
#include <vector>

namespace Gfx
{
    class ShadercIncluder : public shaderc::CompileOptions::IncluderInterface
	{
        struct Data {
            char* buffer1;
            char* buffer2;
        };
	    public:
		    shaderc_include_result* GetInclude(const char* requested_src, shaderc_include_type type, const char* requesting_src, size_t include_depth) override;

		    void ReleaseInclude(shaderc_include_result* data) override;
	};

    struct VertexInputDescription {
        std::vector<VkVertexInputBindingDescription> Bindings;
        std::vector<VkVertexInputAttributeDescription> Attributes;
        VkPipelineVertexInputStateCreateFlags Flags = 0;
    };

    class Pipeline : public Utils::Singleton<Pipeline>
    {
        public:
            Material* GetMaterial(const std::string& name);
            const std::vector<std::string>& GetMaterialNames() { return MaterialNames; } 
            void Build();

            void CompileShader(const std::string& material, const std::string& name, shaderc_shader_kind kind, std::string& data);

        private:
        	VkPipeline Pipeline;
	        VkPipelineLayout PipelineLayout;
            
            VkPipeline ComputePipeline;
	        VkPipelineLayout ComputeLayout; 

        	std::vector<VkPipelineShaderStageCreateInfo> ShaderStages;
	        VertexInputDescription 					VertexDescription;
            VkPipelineVertexInputStateCreateInfo 	VertexInputInfo;
            VkPipelineInputAssemblyStateCreateInfo 	InputAssembly;
            VkViewport 								Viewport;
            VkViewport 								ViewportShadows;
            VkRect2D 								Scissor;
            VkRect2D 								ScissorShadows;
            VkPipelineRasterizationStateCreateInfo 	Rasterizer;
            VkPipelineColorBlendAttachmentState 	ColorBlendAttachment;
            VkPipelineMultisampleStateCreateInfo 	Multisampling;
            VkPipelineDepthStencilStateCreateInfo 	DepthStencil;

            void BuildMaterial(const std::string& material, VkShaderModule* vertexshader, const std::string& vertname, VkShaderModule* fragshader, const std::string& fragname, Gfx::RenderTargetsList id, bool iscompute = false);
            bool LoadShader(const std::string& buffer, VkShaderModule* outshadermodule);

            bool LoadShader(const char* filepath, VkShaderModule* outshadermodule);
            Material* CreateMaterial(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name);
            void Setup(Gfx::RenderTargetsList id);
            void SetupCompute(VkShaderModule computeshader);
            void GetVertexDescription(bool iscompute = false, bool isskinningin = false );
            VkPipelineVertexInputStateCreateInfo VertexInputStageCreateInfo(bool iscompute = false, bool isskinningin = false);
            
            std::vector<std::string> MaterialNames;
            std::unordered_map<std::string,Material> Materials;
    };

}
