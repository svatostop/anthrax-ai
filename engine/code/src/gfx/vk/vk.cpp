module;
#include "aai/io/win_defines.h"
#include "aai/gfx/vk/backend/vk_defines.h"
#include "glm/ext/matrix_transform.hpp"
#include <string.h>

module aai.gfx.vk;
import aai.gfx.vk.buffer;
import aai.gfx.vk.device.helper;
import aai.gfx.vk.frames;
import aai.gfx.vk.rt;
import aai.gfx.vk.loader.texture;
import aai.gfx.vk.rt.helper;
import aai.gfx.vk.rt.cmd;
import aai.utils;
import std;

void vk::base::init(bool validate, GLFWwindow* glfw_win, Display* di, Window w, std::shared_ptr<keeper::camera> c)
{
    cam = c;

    inst.init(validate);
#ifdef AAI_LINUX
    dev.init_linux_surface(inst.get_instance(), glfw_win, di, w);
#else 
    dev.init_windows_surface();
#endif
    dev.init(validate, inst.get_layers());
    window_size = dev.get_window_size();
    cam->set_window_size(window_size);

    frame.init(dev.get_device(), dev.get_graphics_index(), dev.get_swapchains_amount());

    gpu_mem.init(dev.get_devices());
    pipe.set_layout(gpu_mem.get_bindless_layout());

    render.set_window_size(window_size);
    render.init(inst.get_instance(), dev.get_devices(), gpu_mem.get_bindless_set(), gpu_mem.get_buffer_address(gpu_data_type::CAMERA), gpu_mem.get_buffer_address(gpu_data_type::INSTANCE));

    init_rt_states();

    auto f = std::bind(&vk::frames::submit, &frame, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3);
    vk::buffer::set_submit_callback(f, dev.get_queue(vk::queues::type::GRAPHICS));
}

bool vk::base::begin_frame()
{
    if (!frame.sync_frames(dev.get_device(), dev.get_swapchain()))
        on_resize();
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
    if (!frame.submit_and_present(dev.get_queue(vk::queues::type::GRAPHICS), dev.get_swapchain()))
        on_resize();
}

void vk::base::execute()
{
    // todo : multithreading ENGINEEEE
    // todo : imgui
    // todo : json integration
    cam_data.view = cam->get_view();
    cam_data.proj = cam->get_reverse_proj();
    // do something about it ?? todo
    inst_data.clear();
    for (const rq::data& data : rq) {
        instance_data d{};
        d.model = glm::translate(data.model_matrix, glm::vec3(0.0, -1.0, 1.0));
        inst_data.push_back(d); 
    }
    gpu_mem.update(dev.get_devices(), cam_data, inst_data);

    render.refresh_state();
    for (const rq::data& data : rq) {
        // todo - the animations should be updated separately from the mesh object, since different models can have the same animation
        if (data.mesh_handle)
            data.mesh_handle->update_animation(dev.get_devices());
        set_debug_render_pass_name(frame.get_cmd(),  data.tag);
        render.block(frame.get_cmd(), data);
        unset_debug_render_pass_name(frame.get_cmd());
    }
}

void vk::base::create_model(const char* path, std::shared_ptr<model::base> m)
{
    m->load(path, dev.get_devices());
    set_debug_name(path, reinterpret_cast<uint64_t>(m->get_vertex_device_memory()), VK_OBJECT_TYPE_DEVICE_MEMORY);
    set_debug_name(path, reinterpret_cast<uint64_t>(m->get_vertex_buffer()), VK_OBJECT_TYPE_BUFFER);
    set_debug_name(path, reinterpret_cast<uint64_t>(m->get_index_device_memory()), VK_OBJECT_TYPE_DEVICE_MEMORY);
    set_debug_name(path, reinterpret_cast<uint64_t>(m->get_index_buffer()), VK_OBJECT_TYPE_BUFFER);
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

void vk::base::on_resize()
{
    window_size = dev.on_resize();
    render.set_window_size(window_size);
    cam->set_window_size(window_size);
    render.recreate_rts(dev.get_devices());
    init_rt_states();
}
void vk::base::init_rt_states()
{
    std::vector<rt::helper::val> rt_transition_values;
    const rt::base::ref_map ref_map = render.get_rt_ref_map();
    rt_transition_values.reserve(ref_map.size());
    for (const auto& it : ref_map) {
        for (const auto type : it.second.color_types) {
            if (std::find(rt_transition_values.begin(), rt_transition_values.end(), type.v) == rt_transition_values.end()) {
                frame.submit(dev.get_device(), dev.get_queue(vk::queues::type::GRAPHICS), [&](VkCommandBuffer cmd) {
                        render.get_rt(type.v)->memory_barrier(cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
                });
                rt_transition_values.push_back(type.v);
            }

            set_debug_name(rt::helper::get_value(type.v), reinterpret_cast<uint64_t>(render.get_rt(type.v)->get_device_memory()), VK_OBJECT_TYPE_DEVICE_MEMORY);
            set_debug_name(rt::helper::get_value(type.v), reinterpret_cast<uint64_t>(render.get_rt(type.v)->get_image()), VK_OBJECT_TYPE_IMAGE);
        }
        if (it.second.depth_count > 0) {
            if (std::find(rt_transition_values.begin(), rt_transition_values.end(), it.second.depth_types.v) == rt_transition_values.end()) {
                frame.submit(dev.get_device(), dev.get_queue(vk::queues::type::GRAPHICS), [&](VkCommandBuffer cmd) {
                        render.get_rt(it.second.depth_types.v)->memory_barrier(cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
                });
                rt_transition_values.push_back(it.second.depth_types.v);
            }
            set_debug_name(rt::helper::get_value(it.second.depth_types.v), reinterpret_cast<uint64_t>(render.get_rt(it.second.depth_types.v)->get_device_memory()), VK_OBJECT_TYPE_DEVICE_MEMORY);
            set_debug_name(rt::helper::get_value(it.second.depth_types.v), reinterpret_cast<uint64_t>(render.get_rt(it.second.depth_types.v)->get_image()), VK_OBJECT_TYPE_IMAGE);
        }
    }
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

