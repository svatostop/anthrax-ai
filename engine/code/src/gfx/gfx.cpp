#include "aai/io/win_defines.h"
#include <cstdint>
#include <stdio.h>

import aai.gfx.vk.rt.helper;
import aai.gfx;
import glm;
import std;

void gfx::base::init(Display* di, Window w)
{
    vk.init(true, di, w);
}

void gfx::base::run()
{
    if (vk.begin_frame()) {
        vk.execute(); 
        vk.end_frame();
    } 
}

void gfx::base::populate()
{
    uint32_t texture_id = create_texture("./textures/kote-v-bote.jpg");

    glm::vec4 viewport = glm::vec4(800,600,0,0); 

    std::vector<mat::shader_module> shaders;
    shaders.push_back({mat::SHADER_VERT, "./shaders/quad.vert"}),
    shaders.push_back({mat::SHADER_FRAG, "./shaders/quad.frag"}),
    material_pallet.add_material_info({
        .name = "test",
        .rt_ref = vk.get_attachment_ref(rt::base::name::ONE_QUAD),
        .viewport = viewport,
        .scissor = viewport,
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
        .bind_texture = texture_id > 0
    });
    vk.create_material(material_pallet);

    vk.set_rq({
            .tag = "test",
            .material_handle = material_pallet.get("test"),
            .texture_id = texture_id,
    });
}

uint32_t gfx::base::create_texture(const char* path)
{
    auto future = asset_mng.load_async(path, [&](std::shared_ptr<rt::render_target> r, const char*){
        vk.create_texture(path, r);
    });
    uint32_t id = future.get();
    printf("texture value !!! %d\n", id);
    return id;
    // auto future = asset_mng.load_async(path, [&](const std::string&) {  
    //     //vk.create_texture(path);
    //     return nullptr;
    // });
}
