module;
#include "aai/gfx/vk/backend/vk_defines.h"
#include <vulkan/vulkan_core.h>

export module aai.gfx.materials;
export import aai.gfx.materials.types;
import aai.gfx.vk.device;
import aai.json;
import glm;
import std;
export {
    namespace mat {
        struct data {
            std::string name;
            VkPipelineLayout pipeline_layout;
            VkPipeline pipeline;
            rt::base::ref attachment_ref;
            bool dynamic_viewport;

            void clean(VkDevice dev) {
                vkDestroyPipelineLayout(dev, pipeline_layout, nullptr);
                vkDestroyPipeline(dev, pipeline, nullptr);
            }
        };
         
        class materials {
            public:
                void parse_data() {
                    aai::json::parse_material_data(infos_map);
                }
                uint32_t set_data(const std::string& n, VkPipeline pipe, VkPipelineLayout pipe_layout, const rt::base::ref& r, bool dynamic_viewport) {
                    std::shared_ptr<data> d(new data);
                    d->pipeline = pipe; 
                    d->pipeline_layout = pipe_layout; 
                    d->attachment_ref = r;
                    d->dynamic_viewport = dynamic_viewport;
                    d->name = n;
                    ids++;
                    mat_map[ids] = d;
                    return ids;
                }
                const info_helper& get_info(const std::string& name) { return infos_map[name]; }
                void request_texture_use(const std::string& name, bool use) { infos_map[name].bind_texture = use; }
                void request_rt_ref_change(const std::string& name, const rt::base::ref ref) { infos_map[name].rt_ref = ref; }
                rt::name::val get_rt_ref_val(const std::string& name) { return infos_map[name].rt_ref_val;  }
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
                std::map<uint32_t, std::shared_ptr<data>> mat_map;
                uint32_t ids = 0;;
        };
    }
};

