#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <iostream>

namespace header_utils {
inline void ASSERT(bool x, const std::string& str) {
            	bool err = x;												
            	if (err)                                                   	
            	{                                                           
            		std::string errstr = "Error: " + str;					
            		errstr += "\n\n";										
            		throw std::runtime_error(errstr);						
            	}                                                           
        }
inline void CHECK(bool x, const std::string& str) {
            	bool err = x;												
            	if (err)                                                   	
            	{                                                           
            		std::string errstr = "Error: " + str;					
            		errstr += "\n\n"; 
                    std::cout << errstr;
            	}                                                           
        }
inline void read_file(const std::string& filename, std::vector<char>& buffer)
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
