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

void gfx::base::consume_back_data(std::deque<keeper::back_datas>& d)
{
    for (keeper::back_datas& val : d) {
        vk.push_render_data({
            .tag = val.tag,
            .material_handle = material_pallet.get(val.material_id),
            .texture_id = 0,  
            .mesh_handle = model_mng.get(val.model_id),
            .skin_gpu_addresses = vk.get_skin_gpu_address(val.skin_group)
        });
        d.pop_front();
    }
}

void gfx::base::populate_inst(model::inst& inst)
{
    utils::ASSERT(material_pallet.is_empty(), "gfx::populate(): json_parser was not called!");

    const std::string name = inst.get_requested_model_name().empty() ? "./models/cube.glb" : inst.get_requested_model_name();
    const std::string material_name = inst.get_material_name().empty() ? "material_model" : inst.get_material_name();

    uint32_t model_id = create_model(name);
   
    utils::job_sys::wait();

    material_pallet.request_mesh_use(material_name, true);
    utils::ASSERT(!model_mng.get(model_id), "-");

    uint32_t skin_id = model_mng.get(model_id)->is_animated() ? vk.get_skin_id() : 0;
    inst.init(model_mng.get(model_id), skin_id);
            
    material_pallet.request_mesh_animation(material_name, model_mng.get(model_id)->is_animated());
    material_pallet.request_rt_ref_change(material_name, vk.get_attachment_ref(material_pallet.get_rt_ref_val(material_name)));
    uint32_t mat_id = vk.create_material(material_pallet, material_name);
    
    inst.set_model_id(model_id);
    //inst.set_texture_id();
    inst.set_material_id(mat_id);
}
void gfx::base::populate_base()
{
    utils::ASSERT(material_pallet.is_empty(), "gfx::populate(): json_parser was not called!");
    // todo better rq setup
    uint32_t texture_id = create_texture("./textures/kote-v-bote.jpg");

    glm::vec4 viewport = glm::vec4(window_size.x, window_size.y ,0,0); 
    
    material_pallet.request_texture_use("material_sprite", true);
    material_pallet.request_rt_ref_change("material_sprite", vk.get_attachment_ref(material_pallet.get_rt_ref_val("material_sprite")));
    uint32_t sprite_mat_id = vk.create_material(material_pallet, "material_sprite");
    
    // vk.push_rq({
    //         .tag = "test",
    //         .material_handle = material_pallet.get(sprite_mat_id),
    //         .texture_id = texture_id,
    // });

    material_pallet.request_rt_ref_change("material_grid", vk.get_attachment_ref(material_pallet.get_rt_ref_val("material_grid")));
    sprite_mat_id = vk.create_material(material_pallet, "material_grid");
    vk.push_static_render_data({
            .tag = "grid",
            .material_handle = material_pallet.get(sprite_mat_id),
            .texture_id = 0,
            .skin_gpu_addresses = vk.get_skin_gpu_address(0)
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
