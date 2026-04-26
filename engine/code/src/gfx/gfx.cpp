#include "aai/io/win_defines.h"

import aai.gfx.attachments;
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
       //vk.render(); 
        vk.end_frame();
    } 
}

void gfx::base::populate()
{
    glm::vec4 viewport = glm::vec4(800,600,0,0); 

    std::vector<mat::shader_module> shaders;
    shaders.push_back({mat::SHADER_VERT, "./shaders/quad.vert"}),
    shaders.push_back({mat::SHADER_FRAG, "./shaders/quad.frag"}),
    material_pallet.add_material_info({
        .name = "test",
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
        .vertex_attributes = false
    });
    vk.create_material(material_pallet, rt::attachments::get_ref(rt::attachments::name::ONE_QUAD));

    vk.set_rq({
            .tag = "test",
            .attachments = rt::attachments::get_ref(rt::attachments::name::ONE_QUAD),
            .material_handle = material_pallet.get("test"),
    });
}

void gfx::base::create_texture(const char* path)
{
    auto future = asset_mng.load_async(path, [&](const std::string&) {  
        //vk.create_texture(path);
        return nullptr;
    });

    // material_mng.load_async(shaders);
    // rq r = {
    //     .add_resource(handle_id),
    //     .add_material(material_mng.get(TETS)),
    //
    // }
}
