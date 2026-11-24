module;
#include "aai/gfx/vk/model/model_types.h"
#include "aai/io/win_defines.h"
#include "imgui.h"
#include <cstdint>
#include <stdio.h>

module aai.gfx;
import aai.gfx.vk.rt.helper;
import aai.gfx.vk.model;
import aai.utils;
import aai.io;
import aai.utils.threads.job_sys;
import glm;
import std;
void gfx::base::init(GLFWwindow* glfw_win, Display* di, Window w)
{
    vk.init(true, glfw_win, di, w, cam);
    window_size = vk.get_window_size();
}

// TODO MOVE
bool full_word_fit(ui::font& ft, int i , float scale,float x, float size_x) {
    for (; i < ft.get_context()->text.size(); i++) {
        char str_c = ft.get_context()->text[i];
        const ui::font::character& ch = ft.get_char("RaelaGrotesqueExtraLight.ttf", str_c);
        float xpos = x + ch.bearing.x * scale;
        if (str_c == ' ')
            break;
        if ((xpos > size_x)) {
            return false;
        }
        x += (ch.advance >> 6) * scale;
    }
    return true;
}

glm::vec2 measure(ui::font& ft, int i, int start_x)
{
    glm::vec2 dim = glm::vec2(0);
    char prev_c = '1';
    for (; i < ft.get_context()->text.size(); i++) {
        char str_c = ft.get_context()->text[i];
        const ui::font::character& ch = ft.get_char("RaelaGrotesqueExtraLight.ttf", str_c);
if ((prev_c == ' ' || prev_c == '\t') && !full_word_fit(ft, i, 1.0f, dim.x, ft.get_context()->size_x))
    break;
        if (str_c != ' ')
            dim.x += (ch.advance >> 6);
        // if ((prev_c == ' ' || prev_c == '\t') && !full_word_fit(ft, i, scale, x, size_x)) {
        //
        //     dim.y += std::max(ch.size.y, ch.size.y + ch.bearing.y);
        // }
// dim.x += (ch.advance >> 6); 
        prev_c = str_c;
    }
    return dim;
}

void gfx::base::run(float delta_ms)
{
    // TODO MOVE
    // TODO align center
    // TODO move with mouse
    float scale = ft.get_context()->scale;//1.0f;
    float x = ft.get_context()->pos.x;
    if (ft.get_context()->align == ui::alignment::val::CENTER) {
        x = ft.get_context()->pos.x + ((ft.get_context()->size_x + 16 - measure(ft,0, x).x) / 2.0f) ;
    }
    float y = ft.get_context()->pos.y;
    float size_x = x + ft.get_context()->size_x;
    float size_y = 36;
    char prev_c = '1';
    bool next_line = false;
    for (int i = 0; i < ft.get_context()->text.size(); i++) {
        char str_c = ft.get_context()->text[i];
        const ui::font::character& ch = ft.get_char("RaelaGrotesqueExtraLight.ttf", str_c);
        float xpos = x + ch.bearing.x * scale;
        if ((prev_c == ' ' || prev_c == '\t') && !full_word_fit(ft, i, scale, x, size_x)) {
            x = ft.get_context()->pos.x;
            if (ft.get_context()->align == ui::alignment::val::CENTER) {
        x = ft.get_context()->pos.x + ((ft.get_context()->size_x + 16 -  measure(ft,i, ft.get_context()->pos.x).x) / 2.0f);

    }
            xpos = x + ch.bearing.x * scale;
            y +=  ch.px_face_size * scale;
            size_y += 36;
        }

        float baseline = y + ch.px_face_size * scale;  
        float ypos = baseline - ch.bearing.y * scale;

        // std::cout << ch.size.y  << "----:"<< ch.bearing.y << '\n';
        // std::cout << "y-pos: [" << str_c << "] :" << ypos << '\n';
        float w = ch.size.x * scale;
        float h = ch.size.y * scale;
        if (w > 0 && h > 0) {
            vk.push_instanced_rect_data({
                .position = glm::vec2(xpos, ypos),
                .size = glm::vec2(w, h),
                .uv = ch.uv,
                .texture_id = ch.atlas_id,
            });
        }


        prev_c = str_c;
        x += (ch.advance >> 6) * scale;
    }
    if (aai::io::mouse::clicked_rect(ft.get_context()->pos.x, ft.get_context()->pos.y, ft.get_context()->size_x, size_y)) {
        if (aai::io::mouse::is_state(aai::io::mouse::val::LEFT_PRESSED) && aai::io::mouse::was_moved())
        {
            glm::vec2 pos_mouse = aai::io::mouse::get_delta();
            ft.get_context()->pos -= pos_mouse;
        }
        vk.push_instanced_rect_data({
            .position = ft.get_context()->pos,
            .size = glm::vec2(ft.get_context()->size_x + 16, size_y + 16),
            .uv = { 0, 0, 1, 1 },
            .texture_id = 0,
            .has_outline = 1,
        });
    }

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
            .skin_gpu_addresses = vk.get_skin_gpu_address(val.skin_group),
            .model_matrix = val.model_matrix
        });
        d.pop_front();
    }
}

void gfx::base::watch(aai::editor& editor)
{
    editor.watch(utils::editor_types::FONT, ft.get_context()); 
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
    // todo make texture id be actually same as vk texture bind
    // uint32_t texture_id = create_texture("./textures/kote-v-bote.jpg");
    // utils::job_sys::wait();
    
    create_font("RaelaGrotesqueExtraLight.ttf");

    glm::vec4 viewport = glm::vec4(window_size.x, window_size.y ,0,0); 
    
    material_pallet.request_texture_use("material_instanced_rect", true);
    material_pallet.request_mesh_use("material_instanced_rect", true);
    material_pallet.request_vertex_type("material_instanced_rect", mat::vertex_type::INSTANCED_RECT);
    material_pallet.request_rt_ref_change("material_instanced_rect", vk.get_attachment_ref(material_pallet.get_rt_ref_val("material_instanced_rect")));
    uint32_t sprite_mat_id = vk.create_material(material_pallet, "material_instanced_rect");

    {
        std::vector<model::types::vertex> verts(6);
        verts[0].position = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
        verts[0].uv = glm::vec2(0.0f, 0.0f);
        verts[1].position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        verts[1].uv = glm::vec2(0.0f, 1.0f);
        verts[2].position = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
        verts[2].uv = glm::vec2(1.0f, 0.0f);
        verts[3].position = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
        verts[3].uv = glm::vec2(1.0f, 0.0f);
        verts[4].position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        verts[4].uv = glm::vec2(0.0f, 1.0f);
        verts[5].position = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        verts[5].uv = glm::vec2(1.0f, 1.0f);
        
        uint32_t instanced_rect_id = create_mesh("instanced_rect", verts, {});
        vk.initialize_instanced_rect_data({
            .material_handle = material_pallet.get(sprite_mat_id),
            .mesh_handle = model_mng.get(instanced_rect_id),
        }); 
    }
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

    material_pallet.request_rt_ref_change("material_debug_draw_point", vk.get_attachment_ref(material_pallet.get_rt_ref_val("material_debug_draw_point")));
    sprite_mat_id = vk.create_material(material_pallet, "material_debug_draw_point");
    vk.push_debug_render_data({
            .tag = "debug_draw_point",
            .material_handle = material_pallet.get(sprite_mat_id),
            .texture_id = 0,
            .skin_gpu_addresses = vk.get_skin_gpu_address(0),
    });
    material_pallet.request_rt_ref_change("material_debug_draw_line", vk.get_attachment_ref(material_pallet.get_rt_ref_val("material_debug_draw_line")));
    sprite_mat_id = vk.create_material(material_pallet, "material_debug_draw_line");
    vk.push_debug_render_data({
            .tag = "debug_draw_line",
            .material_handle = material_pallet.get(sprite_mat_id),
            .texture_id = 0,
            .skin_gpu_addresses = vk.get_skin_gpu_address(0),
    });
}

void gfx::base::clean_resources()
{
    vk.clean_rts();
    asset_mng.unload_all(vk.get_devices());
    material_pallet.clean(vk.get_devices());
}

void gfx::base::create_font(const std::string& path)
{
    uint32_t id = asset_mng.load(path, [&, path](std::shared_ptr<rt::render_target> r, const std::string&){
        vk.create_atlas_texture(path.c_str(), r, 512, 512);
    });
    utils::job_sys::wait();

    ft.load(path, id, [&](unsigned int w, unsigned int h, unsigned char* b) {
        return vk.add_atlas_texture(asset_mng.get(id), w, h, b);
    });
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

uint32_t gfx::base::create_mesh(const std::string& path, const std::vector<model::types::vertex>& vertex_buffer, const std::vector<uint16_t>& index_buffer)
{
    uint32_t id = model_mng.load(path, [&, path, vertex_buffer, index_buffer](std::shared_ptr<model::base> m, const std::string&) {
        vk.create_mesh(path.c_str(), m, vertex_buffer, index_buffer);
    });
    utils::job_sys::wait();
    return id;

}

