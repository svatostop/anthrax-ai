module;
#include "aai/gfx/vk/backend/vk_defines.h"
#include <stdio.h>

export module aai.gfx.vk.loader.texture;
export import aai.gfx.vk.loader.texture.lib;

import aai.utils;
import aai.gfx.vk.buffer;
import aai.gfx.vk.device;
import aai.gfx.vk.rt;
import std;
export {
    namespace loader {
        namespace texture {
            void load(std::shared_ptr<rt::render_target> target, const char* path, const vk::device::handlers& dev, vk::buffer::handlers& stagingbuffer) {
                int width, height, channels;
                void* pixels = nullptr;
                pixels = loader::texture::load_stbi(path, width, height, channels);
                VkDeviceSize imagesize = width * height * 4;

                utils::ASSERT(!pixels, "failed to load texture image!");

                vk::buffer::allocate(stagingbuffer, dev, imagesize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                vk::buffer::map_memory(stagingbuffer, dev.dev, imagesize, 0, pixels);

                loader::texture::unload_stbi(pixels);
        
                target->set_name(path);
                target->set_format(VK_FORMAT_R8G8B8A8_SRGB);
                target->set_dimensions({ width, height });
                target->set_sampler(true);
                target->create(dev);
            }
        }
    }
};
