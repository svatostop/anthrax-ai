#pragma once
#include "aai/utils/utils.h"
#include "aai/gfx/vk/backend/vk_defines.h"
#include <shaderc/shaderc.hpp>
#include <cstring>
#include <fstream>

class shaderc_incl : public shaderc::CompileOptions::IncluderInterface
{
    struct Data {
        char* buffer1;
        char* buffer2;
    };
    public:
	    shaderc_include_result* GetInclude(const char* requested_src, shaderc_include_type type, const char* requesting_src, size_t include_depth) override
        {
        	std::string name("./shaders/" + std::string(requested_src));

        	char *nameb = new char[name.size()];
        	memcpy(nameb, name.c_str(), name.size());
        
        	std::vector<char> contents;
            header_utils::read_file(name, contents);
        
        	char *contentb = new char[contents.size()];
        	memcpy(contentb, contents.data(), contents.size());
        
        	return new shaderc_include_result {
        		nameb,
        		name.size(),
        		contentb,
        		contents.size(),
        		new Data { nameb, contentb }
        	};
        
        }

	    void ReleaseInclude(shaderc_include_result* data) override
        {
        	Data* fdata = reinterpret_cast<Data*>(data->user_data);
        	delete[] fdata->buffer1;
        	delete[] fdata->buffer2;
        	delete fdata;
        
            }
};

