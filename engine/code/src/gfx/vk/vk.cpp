#include "aai/io/win_defines.h"
#include "aai/gfx/vk/backend/vk_defines.h"
#include <cstdint>
#include <string.h>
#include <stdio.h>

import aai.gfx.vk;
import aai.gfx.vk.buffer;
import aai.gfx.vk.device.helper;
import aai.gfx.vk.frames;
import aai.gfx.vk.rt;
import aai.gfx.vk.loader.texture;
import aai.gfx.vk.rt.helper;
import aai.gfx.vk.rt.cmd;
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

    frame.init(dev.get_device(), dev.get_graphics_index(), dev.get_swapchains_amount());

    gpu_mem.init(dev.get_devices());
    pipe.set_layout(gpu_mem.get_bindless_layout());

    render.init(inst.get_instance(), dev.get_devices(), gpu_mem.get_bindless_set());

    frame.submit(dev.get_device(), dev.get_queue(vk::queues::type::GRAPHICS), [&](VkCommandBuffer cmd) {
        render.get_rt(rt::helper::val::MAIN_COLOR)->memory_barrier(cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    });
    set_debug_name("test", reinterpret_cast<uint64_t>(render.get_rt(rt::helper::val::MAIN_COLOR)->get_device_memory()), VK_OBJECT_TYPE_DEVICE_MEMORY);
    set_debug_name("test", reinterpret_cast<uint64_t>(render.get_rt(rt::helper::val::MAIN_COLOR)->get_image()), VK_OBJECT_TYPE_IMAGE);
}

bool vk::base::begin_frame()
{
    frame.sync_frames(dev.get_device(), dev.get_swapchain());
    return frame.is_swapchain_index_valid();
}

void vk::base::end_frame()
{
    frame.prepare_for_present(
        get_last_target()->get_image(),
        get_last_target()->get_size(),
        dev.get_swapchain_image(frame.get_swapchain_index()),
        { dev.get_swapchain_size().width ,  dev.get_swapchain_size().height }
    );
    frame.submit_and_present(dev.get_queue(vk::queues::type::GRAPHICS), dev.get_swapchain());
}

void vk::base::execute()
{
    set_debug_render_pass_name(frame.get_cmd(), "test");
    render.block(frame.get_cmd(), rq);
    unset_debug_render_pass_name(frame.get_cmd());
}


void vk::base::create_texture(const char* path, std::shared_ptr<rt::render_target> target)
{
    vk::buffer::handlers stagingbuffer;
    loader::texture::load(target, path, dev.get_devices(), stagingbuffer);

    set_debug_name(path, reinterpret_cast<uint64_t>(target->get_device_memory()), VK_OBJECT_TYPE_DEVICE_MEMORY);
    set_debug_name(path, reinterpret_cast<uint64_t>(target->get_image()), VK_OBJECT_TYPE_IMAGE);

    frame.submit(dev.get_device(), dev.get_queue(vk::queues::type::GRAPHICS), [&](VkCommandBuffer cmd) {
        target->memory_barrier(cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    });
    frame.submit(dev.get_device(), dev.get_queue(vk::queues::type::GRAPHICS), [&](VkCommandBuffer cmd) {
        target->copy(cmd, stagingbuffer.buffer, static_cast<uint32_t>(target->get_size().x), static_cast<uint32_t>(target->get_size().y));
    });
    frame.submit(dev.get_device(), dev.get_queue(vk::queues::type::GRAPHICS), [&](VkCommandBuffer cmd) {
        target->memory_barrier(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    });
    vkDestroyBuffer(dev.get_device(), stagingbuffer.buffer, nullptr);
    vkFreeMemory(dev.get_device(), stagingbuffer.device_memory, nullptr);

    gpu_mem.update_texture(dev.get_device(), target->get_name(), target->get_image_view(), *(target->get_sampler()));
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

