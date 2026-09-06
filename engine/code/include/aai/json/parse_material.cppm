module;
#include <stdio.h>

export module aai.json.materials;
export import aai.json.helper;
import aai.gfx.materials.types; 

import std;

export {
    namespace aai {
        namespace json {
            
            mat::rasterizer_helper parse_rasterizer(const std::string& entry) {
                std::ifstream f("materials/helpers/rasterizer.json");
                njson data = njson::parse(f);
                njson d = data[entry];
                // todo make checks whether the entry acually exist
                mat::rasterizer_helper helper; 
                helper.polygon = mat::polygon::get_key(get<std::string>(d, aai::json::val::POLYGON, "entry_not_found").c_str());
                helper.cull = mat::cull::get_key(get<std::string>(d,aai::json::val::CULL , "entry_not_found").c_str());        
                helper.face = mat::face::get_key(get<std::string>(d,aai::json::val::FACE , "entry_not_found").c_str());        
                f.close();                                                                
                return helper;
            }
            mat::color_blend_helper parse_color_blend(const std::string& entry) {
                std::ifstream f("materials/helpers/color_blends.json");
                njson data = njson::parse(f);
                njson d = data[entry];
                // todo make checks whether the entry acually exist

                mat::color_blend_helper helper; 
                helper.src_color_val = mat::color_blends::get_key(get<std::string>(d, aai::json::val::SRC_COLOR, "entry_not_found").c_str());
                helper.dst_color_val = mat::color_blends::get_key(get<std::string>(d, aai::json::val::DST_COLOR, "entry_not_found").c_str());
                helper.src_alpha_val = mat::color_blends::get_key(get<std::string>(d, aai::json::val::SRC_ALPHA, "entry_not_found").c_str());
                helper.dst_alpha_val = mat::color_blends::get_key(get<std::string>(d, aai::json::val::DST_ALPHA, "entry_not_found").c_str());
                helper.c_op = mat::color_op::get_key(get<std::string>(d, json::val::C_OP, "entry_not_found").c_str()); 
                helper.a_op = mat::color_op::get_key(get<std::string>(d, json::val::A_OP, "entry_not_found").c_str()); 
                helper.alpha_blend = json::get<bool>(d, json::val::BLEND, false);               
                f.close();                                                               
                return helper;
            }
            mat::depth_helper parse_depth_stencil(const njson& d)
            {
                mat::depth_helper helper;
                helper.d_op = mat::depth_op::get_key(get<std::string>(d, json::val::DEPTH_OP , "entry_not_found").c_str());
                helper.depth_write = json::get<bool>(d, json::val::DEPTH_WRITE, false);             
                helper.depth_test = json::get<bool>(d, json::val::DEPTH_TEST, false);
                return helper;
            }
            void parse_shaders(const njson& d, std::vector<mat::shader_module>& shaders) {
                shaders.push_back({mat::SHADER_VERT, "./shaders/" + get<std::string>(d, json::val::VERTEX, "entry_not_found" )});
                shaders.push_back({mat::SHADER_FRAG, "./shaders/" + get<std::string>(d, json::val::FRAGMENT, "entry_not_found" )});
            }
            void parse_pipeline(mat::info_helper& helper, const std::string& entry) {
                std::ifstream f("materials/helpers/pipeline.json");
                njson data = njson::parse(f);
                njson d = data[entry];
                
                helper.rt_ref_val = rt::name::get_key(get<std::string>(d, json::val::RT_REF, "entry_not_found").c_str());
                helper.viewport = glm::vec4(get_vec2(d, json::val::VIEWPORT), 0, 0);
                helper.scissor = glm::vec4(get_vec2(d, json::val::SCISSOR), 0, 0);
                helper.dynamic_viewport = json::get<bool>(d, json::val::DYNAMIC_VIEWPORT, false);
                helper.rasterizer = parse_rasterizer(get<std::string>(d, json::val::RASTERIZER, "entry_not_found" ));         
                helper.color_blend = parse_color_blend(get<std::string>(d, json::val::COLOR_BLEND , "entry_not_found"));;
                helper.depth_stencil = parse_depth_stencil(d);
                helper.multisampling = json::get<bool>(d, json::val::MULTISAMPLING, false);
                helper.vertex_attributes = false;
                helper.bind_texture = false;
                helper.has_bones = false;
            }
            void parse_material_data(mat::material_infos_map& out_data) {
                std::ifstream f("materials/material_pallet.json");
                njson data = njson::parse(f);
                for (auto& d : data.items()) {
                    mat::info_helper helper;
                    helper.name = d.key();
                    njson value = d.value();
                    parse_pipeline(helper, get<std::string>(value, json::val::PIPELINE_REF, "entry_not_found"));
                    parse_shaders(value, helper.shaders);
                    out_data[d.key()] = helper;
                }
                f.close();
            }
        }
    } 
};
