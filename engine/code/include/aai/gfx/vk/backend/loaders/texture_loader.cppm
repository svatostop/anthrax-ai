module;
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

export module aai.gfx.vk.loader.texture;

export {
    namespace loader {
        namespace texture {
            void load_stbi(const char* path, void* pixels, int& width, int& height, int& channels)
            {
                pixels = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);
            }
            void unload_stbi(void* pixels) 
            {
                stbi_image_free(pixels);
            }
        }
    }
};
