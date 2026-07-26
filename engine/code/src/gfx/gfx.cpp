module;
#include "aai/io/win_defines.h"
#include <cstdint>
#include <stdio.h>

module aai.gfx;
import aai.gfx.vk.rt.helper;
import aai.gfx.vk.model;
import glm;
import std;

void gfx::base::init(GLFWwindow* glfw_win, Display* di, Window w)
{
    vk.init(true, glfw_win, di, w, cam);
    window_size = vk.get_window_size();
}

void gfx::base::run()
{
    cam->update();

    if (vk.begin_frame()) {
        vk.execute(); 
        vk.end_frame();
    } 
    window_size = vk.get_window_size();
}

void gfx::base::populate()
{
    // todo better rq setup
    uint32_t texture_id = create_texture("./textures/kote-v-bote.jpg");
    uint32_t model_id = create_model("./models/cube.glb");
    model_id = create_model("./models/man.glb");
    
    glm::vec4 viewport = glm::vec4(window_size.x, window_size.y ,0,0); 

    std::vector<mat::shader_module> shaders;
    shaders.push_back({mat::SHADER_VERT, "./shaders/quad.vert"}),
    shaders.push_back({mat::SHADER_FRAG, "./shaders/quad.frag"}),
    material_pallet.add_material_info({
        .name = "test",
        .rt_ref = vk.get_attachment_ref(rt::base::name::ONE_QUAD),
        .viewport = viewport,
        .scissor = viewport,
        .dynamic_viewport = true,
        .rasterizer = {
            .polygon = mat::MODE_FILL,
            .cull = mat::CULL_NONE,
            .face = mat::CC
        },
        .color_blend = {
            .src_color_val = mat::SRC_ALPHA,
            .dst_color_val = mat::ONE_MINUS_SRC_ALPHA, 
            .src_alpha_val = mat::SRC_ALPHA,
            .dst_alpha_val = mat::ONE_MINUS_SRC_ALPHA,
            .c_op = mat::COLOR_OP_ADD,
            .a_op = mat::COLOR_OP_ADD,
            .alpha_blend = false
        },
        .depth_stencil = {
            .depth_write = false,
            .depth_test = false
        },
        .shaders = shaders,
        .multisampling = false,
        .vertex_attributes = false,
        .bind_texture = texture_id > 0,
        .has_bones = false
    });
    vk.create_material(material_pallet);

    vk.push_rq({
            .tag = "test",
            .material_handle = material_pallet.get("test"),
            .texture_id = texture_id,
    });
    
    shaders.clear();
    shaders.push_back({mat::SHADER_VERT, "./shaders/mesh.vert"}),
    shaders.push_back({mat::SHADER_FRAG, "./shaders/mesh.frag"}),
    material_pallet.add_material_info({
        .name = "test_model",
        .rt_ref = vk.get_attachment_ref(rt::base::name::ONE_QUAD),
        .viewport = viewport,
        .scissor = viewport,
        .dynamic_viewport = true,
        .rasterizer = {
            .polygon = mat::MODE_FILL,
            .cull = mat::CULL_NONE,
            .face = mat::CC
        },
        .color_blend = {
            .src_color_val = mat::SRC_ALPHA,
            .dst_color_val = mat::ONE_MINUS_SRC_ALPHA, 
            .src_alpha_val = mat::SRC_ALPHA,
            .dst_alpha_val = mat::ONE_MINUS_SRC_ALPHA,
            .c_op = mat::COLOR_OP_ADD,
            .a_op = mat::COLOR_OP_ADD,
            .alpha_blend = false
        },
        .depth_stencil = {
            .depth_write = false,
            .depth_test = false
        },
        .shaders = shaders,
        .multisampling = false,
        .vertex_attributes = true,
        .bind_texture = false,
        .has_bones = false
    });
    vk.create_material(material_pallet);

    // vk.push_rq({
    //         .tag = "test_model",
    //         .material_handle = material_pallet.get("test_model"),
    //         .texture_id = 0,
    //         .mesh_handle = model_mng.get("./models/cube.glb"),
    // });
    //
    shaders.clear();
    shaders.push_back({mat::SHADER_VERT, "./shaders/mesh_anim.vert"}),
    shaders.push_back({mat::SHADER_FRAG, "./shaders/mesh.frag"}),
    material_pallet.add_material_info({
        .name = "test_model_anim",
        .rt_ref = vk.get_attachment_ref(rt::base::name::ONE_QUAD),
        .viewport = viewport,
        .scissor = viewport,
        .dynamic_viewport = true,
        .rasterizer = {
            .polygon = mat::MODE_FILL,
            .cull = mat::CULL_NONE,
            .face = mat::CC
        },
        .color_blend = {
            .src_color_val = mat::SRC_ALPHA,
            .dst_color_val = mat::ONE_MINUS_SRC_ALPHA, 
            .src_alpha_val = mat::SRC_ALPHA,
            .dst_alpha_val = mat::ONE_MINUS_SRC_ALPHA,
            .c_op = mat::COLOR_OP_ADD,
            .a_op = mat::COLOR_OP_ADD,
            .alpha_blend = false
        },
        .depth_stencil = {
            .depth_write = false,
            .depth_test = false
        },
        .shaders = shaders,
        .multisampling = false,
        .vertex_attributes = true,
        .bind_texture = false,
        .has_bones = true
    });
    // todo push and create of mat - can be joined
    vk.create_material(material_pallet);

    vk.push_rq({
            .tag = "test_model_anim",
            .material_handle = material_pallet.get("test_model_anim"),
            .texture_id = 0,
            // todo - models must be retrieved using unique id from asset mng
            .mesh_handle = model_mng.get("./models/man.glb"),
    });

}

void gfx::base::clean_resources()
{
    vk.clean_rts();
    asset_mng.unload_all(vk.get_devices());
    material_pallet.clean(vk.get_devices());
}

uint32_t gfx::base::create_texture(const char* path)
{
    auto future = asset_mng.load_async(path, [&](std::shared_ptr<rt::render_target> r, const char*){
        vk.create_texture(path, r);
    });
    uint32_t id = future.get();
    printf("texture value !!! %d\n", id);
    return id;
}

uint32_t gfx::base::create_model(const char* path)
{
    auto future = model_mng.load_async(path, [&](std::shared_ptr<model::base> m, const char*) {
        vk.create_model(path, m);
    });
    uint32_t id = future.get();
    printf("model value !!! %d\n", id);
    return id;
}
