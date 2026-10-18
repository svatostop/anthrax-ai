module;
#include "aai/io/win_defines.h"
#include <cstdint>
#include <stdio.h>

module aai.gfx;
import aai.gfx.vk.rt.helper;
import aai.gfx.vk.model;
import aai.utils;
import aai.utils.threads.job_sys;
import glm;
import std;
void gfx::base::init(GLFWwindow* glfw_win, Display* di, Window w)
{
    vk.init(true, glfw_win, di, w, cam);
    window_size = vk.get_window_size();
}

void gfx::base::run(float delta_ms)
{
    if (vk.begin_frame()) {
        vk.execute(delta_ms); 
        vk.end_frame();
    } 
    window_size = vk.get_window_size();
}

void gfx::base::populate()
{
    utils::ASSERT(material_pallet.is_empty(), "gfx::populate(): json_parser was not called!");
    // todo better rq setup
    uint32_t texture_id = create_texture("./textures/kote-v-bote.jpg");
    uint32_t model_cube_id = create_model("./models/cube.glb");
    uint32_t model_fox_id = create_model("./models/fox.glb");
   
    utils::job_sys::wait();

    glm::vec4 viewport = glm::vec4(window_size.x, window_size.y ,0,0); 
    
    material_pallet.request_texture_use("material_sprite", true);
    material_pallet.request_rt_ref_change("material_sprite", vk.get_attachment_ref(material_pallet.get_rt_ref_val("material_sprite")));
    uint32_t sprite_mat_id = vk.create_material(material_pallet, "material_sprite");
    
    // vk.push_rq({
    //         .tag = "test",
    //         .material_handle = material_pallet.get(sprite_mat_id),
    //         .texture_id = texture_id,
    // });

    material_pallet.request_mesh_use("material_model", true);
    utils::ASSERT(!model_mng.get(model_cube_id), "-");
    material_pallet.request_mesh_animation("material_model", model_mng.get(model_cube_id)->is_animated());
    material_pallet.request_rt_ref_change("material_model", vk.get_attachment_ref(material_pallet.get_rt_ref_val("material_model")));
    uint32_t mat_id = vk.create_material(material_pallet, "material_model");
    vk.push_rq({
            .tag = "test_model",
            .material_handle = material_pallet.get(mat_id),
            .texture_id = 0,
            .mesh_handle = model_mng.get(model_cube_id),
    });

        // todo_vulkan return back for anim
    material_pallet.request_mesh_use("material_model", true);
    material_pallet.request_mesh_animation("material_model", false);// model_mng.get(model_fox_id)->is_animated());
    material_pallet.request_rt_ref_change("material_model", vk.get_attachment_ref(material_pallet.get_rt_ref_val("material_model")));
    mat_id = vk.create_material(material_pallet, "material_model");
    vk.push_rq({
            .tag = "test_model_anim",
            .material_handle = material_pallet.get(mat_id),
            .texture_id = 0,
            .mesh_handle = model_mng.get(model_fox_id),
    });

    material_pallet.request_rt_ref_change("material_grid", vk.get_attachment_ref(material_pallet.get_rt_ref_val("material_grid")));
    sprite_mat_id = vk.create_material(material_pallet, "material_grid");
    vk.push_rq({
            .tag = "grid",
            .material_handle = material_pallet.get(sprite_mat_id),
            .texture_id = 0,
    });

}

void gfx::base::clean_resources()
{
    vk.clean_rts();
    asset_mng.unload_all(vk.get_devices());
    material_pallet.clean(vk.get_devices());
}

uint32_t gfx::base::create_texture(const std::string& path)
{
    uint32_t id = asset_mng.load(path, [&, path](std::shared_ptr<rt::render_target> r, const std::string&){
        vk.create_texture(path.c_str(), r);
    });
    printf("texture value !!! %d\n", id);
    return id;
}

uint32_t gfx::base::create_model(const std::string& path)
{
    uint32_t id = model_mng.load(path, [&, path](std::shared_ptr<model::base> m, const std::string&) {
        vk.create_model(path.c_str(), m);
    });
    printf("model value !!! %d\n", id);
    return id;
}
