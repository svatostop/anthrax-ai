module;
#include <vulkan/vk_enum_string_helper.h>

export module aai.utils;
import std;
export {
    namespace utils {
    // LOOKUP TABLES MACRO
    #define GENERATE_ENUM(element, name) element,
    #define GENERATE_STRING(element, name) case element: return name;
    
    #define DECLARE_LOOKUP_TABLE(TABLE_NAME, ENUM_NAME)                                                                    \
    enum ENUM_NAME                                                                                                         \
    {                                                                                                                      \
        TABLE_NAME(GENERATE_ENUM)                                                                                          \
    };                                                                                                                     \
                                                                                                                           \
    inline const std::string get_value(ENUM_NAME id)                                                                             \
    {                                                                                                                      \
        switch (id)                                                                                                        \
        {                                                                                                                  \
            TABLE_NAME(GENERATE_STRING)                                                                               \
        default:                                                                                                           \
            return "undef";                                                                                                \
        }                                                                                                                  \
    }
    //------------------

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

    void read_file(const std::string& filename, std::vector<char>& buffer)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            ASSERT(true, "Can't open a file: " + filename);
        }

        size_t filesize = (size_t) file.tellg();
        buffer.resize(filesize);
        file.seekg(0);
        file.read(buffer.data(), filesize);
        file.close();
    }

    }
};
