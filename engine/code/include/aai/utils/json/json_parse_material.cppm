module;
#include <stdio.h>

export module aai.json;
export import aai.json.helper;
import aai.gfx.materials.types; 

import std;
import nlohmann.json;
import glm;
using njson = nlohmann::json;

export {
    namespace aai {
        namespace json {
            template <typename T>
            T get(const njson& d, json::val e) {
                return d[json::get_value(e)].get<T>();
            }
            glm::vec2 get_vec2(const njson& d, json::val e) {
                glm::vec2 v;
                int i = 0;
                for (auto& k : d) {
                    if (i >= 2)
                        return v;
                    v[i] = k;
                    i++;
                }
                return v;
            }

            mat::rasterizer_helper parse_rasterizer(const std::string& entry) {
                std::ifstream f("materials/helpers/rasterizer.json");
                njson data = njson::parse(f);
                njson d = data[entry];
                // todo make checks whether the entry acually exist
                mat::rasterizer_helper helper; 
                helper.polygon = mat::polygon::get_key(get<std::string>(d, aai::json::val::POLYGON).c_str());
                helper.cull = mat::cull::get_key(get<std::string>(d,aai::json::val::CULL).c_str());        
                helper.face = mat::face::get_key(get<std::string>(d,aai::json::val::FACE).c_str());        
                f.close();                                                                
                return helper;
            }
            mat::color_blend_helper parse_color_blend(const std::string& entry) {
                std::ifstream f("materials/helpers/rasterizer.json");
                njson data = njson::parse(f);
                njson d = data[entry];
                // todo make checks whether the entry acually exist

                mat::color_blend_helper helper; 
                helper.src_color_val = mat::color_blends::get_key(get<std::string>(d, aai::json::val::SRC_COLOR).c_str());
                helper.dst_color_val = mat::color_blends::get_key(get<std::string>(d, aai::json::val::DST_COLOR).c_str());
                helper.src_alpha_val = mat::color_blends::get_key(get<std::string>(d, aai::json::val::SRC_ALPHA).c_str());
                helper.dst_alpha_val = mat::color_blends::get_key(get<std::string>(d, aai::json::val::DST_ALPHA).c_str());
                helper.c_op = mat::color_op::get_key(get<std::string>(d, json::val::C_OP).c_str()); 
                helper.a_op = mat::color_op::get_key(get<std::string>(d, json::val::A_OP).c_str()); 
                helper.alpha_blend = json::get<bool>(d, json::val::BLEND);
                f.close();
                return helper;
            }
            mat::depth_helper parse_depth_stencil(const njson& d)
            {
                mat::depth_helper helper;
                helper.d_op = mat::depth_op::get_key(get<std::string>(d, json::val::DEPTH_OP).c_str());
                helper.depth_write = json::get<bool>(d, json::val::DEPTH_WRITE);
                helper.depth_test = json::get<bool>(d, json::val::DEPTH_TEST);
                return helper;
            }
            void parse_shaders(const njson& d, std::vector<mat::shader_module>& shaders) {
                shaders.push_back({mat::SHADER_VERT, "./shaders/" + get<std::string>(d, json::val::VERTEX)});
                shaders.push_back({mat::SHADER_FRAG, "./shaders/" + get<std::string>(d, json::val::FRAGMENT)});
            }
            void parse_pipeline(mat::info_helper& helper, const std::string& entry) {
                std::ifstream f("materials/helpers/pipeline.json");
                njson data = njson::parse(f);
                njson d = data[entry];
                
                helper.rt_ref_val = rt::name::get_key(get<std::string>(d, json::val::RT_REF).c_str());
                helper.viewport = glm::vec4(json::get_vec2(d, json::val::VIEWPORT), 0, 0);
                helper.scissor = glm::vec4(json::get_vec2(d, json::val::SCISSOR), 0, 0);
                helper.dynamic_viewport = json::get<bool>(d, json::val::DYNAMIC_VIEWPORT);
                helper.rasterizer = parse_rasterizer(get<std::string>(d, json::val::RASTERIZER));         
                helper.color_blend = parse_color_blend(get<std::string>(d, json::val::COLOR_BLEND));;
                helper.depth_stencil = parse_depth_stencil(d);
                helper.multisampling = json::get<bool>(d, json::val::MULTISAMPLING);
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
                    parse_pipeline(helper, get<std::string>(d, json::val::PIPELINE_REF));
                    parse_shaders(d, helper.shaders);
                    out_data[d.key()] = helper;
                //     // printf("val: %s, key: %s\n", d.value().c_str(), d.key().c_str());
                //     std::cout << d.value() << " " << d.key() << "\n";
                }
                f.close();
            }
        }
    } 
};
