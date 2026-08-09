module;
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

export module aai.gfx.vk.loader.texture.lib;

export {
    namespace loader {
        namespace texture {
            stbi_uc* load_stbi(const char* path, int& width, int& height, int& channels)
            {
                stbi_uc* pixels;
                pixels = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);
                return pixels;
            }
            void unload_stbi(void* pixels) 
            {
                stbi_image_free(pixels);
            }
        }
    }
};
