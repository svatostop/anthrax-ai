module;
#include "aai/gfx/vk/backend/loaders/shader_loader_internal.h"

export module aai.gfx.vk.loader.shader;

export {
    namespace loader {
        namespace shader {

            void compile(const std::string& name, shaderc_shader_kind kind, std::string& data)
            {
            	std::vector<char> buffer;
                header_utils::read_file(name, buffer);
            	
                shaderc::Compiler compiler;
            	shaderc::CompileOptions options{};
            	options.SetIncluder(std::make_unique<shaderc_incl>());
        //     	//  	// Like -DMY_DEFINE=1
        //     	//  	// if (material == "skybox")
        //     	//        // options.AddMacroDefinition("SKINNING_IN_DECL");
            	shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(buffer.data(), buffer.size(), kind, name.c_str(), options);
            	header_utils::ASSERT(module.GetCompilationStatus() != shaderc_compilation_status_success, "loader::shader::compile(): " + module.GetErrorMessage());
            	data = std::string(std::string((const char*)module.cbegin(), (const char*)module.cend()));
            }
        }
    }
};

