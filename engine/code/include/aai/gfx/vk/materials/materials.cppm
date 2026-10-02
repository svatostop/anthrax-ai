module;
#include "aai/gfx/vk/backend/vk_defines.h"

export module aai.gfx.materials;
export import aai.gfx.materials.types;
import aai.gfx.vk.device;
import glm;
import std;
export {
    namespace mat {
        class materials {
            public:
                uint32_t set_data(const std::string& n, VkPipeline pipe, VkPipelineLayout pipe_layout, const rt::base::ref& r, bool dynamic_viewport) {
                    material_ids_map::const_iterator it = mat_id_map.find(n);
                    if (it != mat_id_map.end())
                        return it->second;
                    std::shared_ptr<data> d(new data);
                    d->pipeline = pipe; 
                    d->pipeline_layout = pipe_layout; 
                    d->attachment_ref = r;
                    d->dynamic_viewport = dynamic_viewport;
                    d->name = n;
                    ids++;
                    mat_map[ids] = d;
                    mat_id_map[n] = ids;
                    return ids;
                }
                bool is_empty() { return infos_map.empty(); }
                material_infos_map& get_material_info_data() { return infos_map; }
                const info_helper& get_info(const std::string& name) { return infos_map[name]; }
                void request_mesh_use(const std::string& name, bool use) {infos_map[name].vertex_attributes = use;}
                void request_mesh_animation(const std::string& name, bool has_anim) { infos_map[name].has_bones = has_anim;}
                void request_texture_use(const std::string& name, bool use) { infos_map[name].bind_texture = use; }
                void request_rt_ref_change(const std::string& name, const rt::base::ref ref) { infos_map[name].rt_ref = ref; }
                rt::name::val get_rt_ref_val(const std::string& name) { return infos_map[name].rt_ref_val;  }
                
                bool exists(const std::string& name) {
                    material_ids_map::const_iterator it = mat_id_map.find(name);
                    return it != mat_id_map.end();
                }
                uint32_t get_id(const std::string& name) {
                    material_ids_map::const_iterator it = mat_id_map.find(name);
                    if (it != mat_id_map.end())
                        return it->second;
                    return 0;
                }

                std::shared_ptr<data> get(uint32_t id) { 
                	auto it = mat_map.find(id);
                	if (it == mat_map.end()) {
                		return nullptr;
                	}
                	else {
                		return (*it).second;
                	}
                }

                void clean(const vk::device::handlers& dev) {
                    for (auto& m : mat_map) {
                        m.second->clean(dev.dev);
                    }
                }
            private:
                material_infos_map infos_map;
                material_map mat_map;
                material_ids_map mat_id_map;
                uint32_t ids = 0;;
        };
    }
};

