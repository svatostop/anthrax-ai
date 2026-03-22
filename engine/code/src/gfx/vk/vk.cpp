#include "aai/io/win_defines.h"
#include "aai/gfx/vk/backend/vk_defines.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

import aai.gfx.vk;
import aai.gfx.vk.buffer;
import aai.gfx.vk.rt;

void vk::base::init(bool validate, Display* di, Window w)
{
    inst.init(validate);
#ifdef AAI_LINUX
    dev.init_linux_surface(inst.get_instance(), di, w);
#else 
    dev.init_windows_surface();
#endif
    dev.init(validate, inst.get_layers());

    frame.init(dev.get_device(), dev.get_graphics_index());
}

void vk::base::create_texture(const char* path)
{
    int width, height, channels;
    stbi_uc* pixels = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);
    VkDeviceSize imagesize = width * height * 4;

    // std::cout << path << '\n';
    // utils::ASSERT(!pixels, "failed to load texture image!");

    vk::buffer::handlers stagingbuffer;
    vk::buffer::allocate(stagingbuffer, dev.get_devices(), imagesize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vk::buffer::map_memory(stagingbuffer, dev.get_device(), imagesize, 0, pixels);

    stbi_image_free(pixels);

    // rt::render_target texture(path);
    // texture.SetFormat(VK_FORMAT_R8G8B8A8_SRGB);
    // texture.SetDimensions({ width, height });
    //
    // texture.CreateRenderTarget();
    //
    // Submit([&](VkCommandBuffer cmd) {
    //     texture.MemoryBarrier(cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    // });
    // Submit([&](VkCommandBuffer cmd) {
    //     texture.Copy(cmd, stagingbuffer.Buffer, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    // });
    // Submit([&](VkCommandBuffer cmd) {
    //     texture.MemoryBarrier(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    // });
    vkDestroyBuffer(dev.get_device(), stagingbuffer.buffer, nullptr);
    vkFreeMemory(dev.get_device(), stagingbuffer.device_memory, nullptr);


}

