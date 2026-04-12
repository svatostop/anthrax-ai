#include "aai/io/win_defines.h"
#include "aai/gfx/vk/backend/vk_defines.h"
#include <cstdint>
#include <string.h>

import aai.gfx.vk;
import aai.gfx.vk.buffer;
import aai.gfx.vk.device.helper;
import aai.gfx.vk.frames;
import aai.gfx.vk.rt;
import aai.gfx.vk.loader.texture;
import aai.utils;
import std;

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

    gpu_mem.init(dev.get_devices());
    pipe.set_layout(gpu_mem.get_bindless_layout());
}

void vk::base::submit(std::function<void(VkCommandBuffer cmd)>&& func)
{
	VkCommandBuffer cmd = frame.get_upload_cmd();
	VkCommandBufferBeginInfo cmdBeginInfo = frame.cmd_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    utils::VK_ASSERT(vkBeginCommandBuffer(cmd, &cmdBeginInfo), "failed to begin command buffer!");
	func(cmd);
    utils::VK_ASSERT(vkEndCommandBuffer(cmd), "failed to end command buffer!");

	VkSubmitInfo submitinfo = frame.submit_info(&cmd);
    utils::VK_ASSERT(vkQueueSubmit(dev.get_queue(vk::queues::type::GRAPHICS), 1, &submitinfo, *(frame.get_upload_fence())), "failed to submit upload queue!");

	vkWaitForFences(dev.get_device(), 1, frame.get_upload_fence(), true, 9999999999);
	vkResetFences(dev.get_device(), 1, frame.get_upload_fence());
	vkResetCommandPool(dev.get_device(), frame.get_upload_cmd_pool(), 0);
}

void vk::base::create_texture(const char* path)
{
    int width, height, channels;
    void* pixels = nullptr;
    loader::texture::load_stbi(path, pixels, width, height, channels);
    VkDeviceSize imagesize = width * height * 4;

    utils::ASSERT(!pixels, "failed to load texture image!");

    vk::buffer::handlers stagingbuffer;
    vk::buffer::allocate(stagingbuffer, dev.get_devices(), imagesize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vk::buffer::map_memory(stagingbuffer, dev.get_device(), imagesize, 0, pixels);
    
    loader::texture::unload_stbi(pixels);

    rt::render_target texture(path);
    texture.set_format(VK_FORMAT_R8G8B8A8_SRGB);
    texture.set_dimensions({ width, height });
    texture.set_sampler(true);
    texture.create(dev.get_devices());
    
    set_debug_name(path, reinterpret_cast<uint64_t>(texture.get_device_memory()), VK_OBJECT_TYPE_DEVICE_MEMORY);
    
    submit([&](VkCommandBuffer cmd) {
        texture.memory_barrier(cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    });
    submit([&](VkCommandBuffer cmd) {
        texture.copy(cmd, stagingbuffer.buffer, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    });
    submit([&](VkCommandBuffer cmd) {
        texture.memory_barrier(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    });

    vkDestroyBuffer(dev.get_device(), stagingbuffer.buffer, nullptr);
    vkFreeMemory(dev.get_device(), stagingbuffer.device_memory, nullptr);
}

void vk::base::set_debug_name(const std::string& name, uint64_t handle, VkObjectType type)
{
    VkDebugUtilsObjectNameInfoEXT info;
	info.pNext = nullptr;
	info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	info.objectHandle = handle;
	info.objectType = type;
	info.pObjectName = name.c_str();
 
    inst.SetDebugUtilsObjectNameEXT(dev.get_device(), &info);
}

void vk::base::unset_debug_render_pass_name(VkCommandBuffer cmd)
{
    inst.SetEndDebugUtilsLabelEXT(cmd);
}

void vk::base::set_debug_render_pass_name(VkCommandBuffer cmd, const std::string& name)
{
    static float r, g, b = 0.0f;
    static bool r_passed, g_passed, b_passed = false;

    if (!r_passed) {
        r += 0.5f;
        g += 0.2f;
        b += 0.2f;
        r_passed = true;
        b_passed = false;
        g_passed = false;
    }
    else if (!b_passed) {
        b += 0.5f;
        r += 0.2f;
        g += 0.2f;
        b_passed = true;
    }
    else if (!g_passed) {
        g += 0.5f;
        r += 0.2f;
        b += 0.2f;
        g_passed = true;
        r_passed = false;
    }
    r = r >= 1.0f ? 0.0 : r;
    b = b >= 1.0f ? 0.0 : b;
    g = g >= 1.0f ? 0.0 : g;

    VkDebugUtilsLabelEXT label = {};
    label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    float color[4] = { r, g, b, 1.0f };
    memcpy(label.color, &color[0], sizeof(float) * 4);
    label.pLabelName = name.c_str();
    inst.SetBeginDebugUtilsLabelEXT(cmd, &label);
}

