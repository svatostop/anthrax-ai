module;
// #include "aai/gfx/vk/backend/vk_defines.h" 
#ifdef __cplusplus
#undef __cplusplus
#include <vulkan/vk_enum_string_helper.h>
#define __cplusplus
#endif

export module aai.utils;
import std;
export {
    namespace utils {
        template <typename T>
        class singleton
        {
            private:
            protected:
                singleton() {}

            public:
                singleton(const singleton* obj) = delete;
                singleton* operator=(const singleton*) = delete;

                static T* get() {  static T Instance; return &Instance; }
        };

        void ASSERT(bool x, const std::string& str) {
            	bool err = x;												
            	if (err)                                                   	
            	{                                                           
            		std::string errstr = "Error: " + str;					
            		errstr += "\n\n";										
            		throw std::runtime_error(errstr);						
            	}                                                           
        }
        void VK_ASSERT(VkResult err, const std::string& str) {
            	if (err)                                                    
            	{        													
            		std::string vulkan = string_VkResult(err);              
            		std::string errstr = "Vulkan: Error: " + vulkan;		
            		errstr += "\n\n" + str;									
            		throw std::runtime_error(errstr);						
                }
        }
    }
};
