module;
#include <cstdint>
#include <vulkan/vulkan_core.h>
#include "aai/gfx/vk/model/model_types.h"

export module aai.gfx.vk.model.inst;
import aai.gfx.vk.model.anim;
export import aai.gfx.vk.model;
export import aai.gfx.vk.device;
export {
    namespace model {
        class inst {
            public:
                inst() {}
                inst(const std::string& n, const std::string& m, const std::string& t = "") { requested_model = n; material_name = m; if (t.empty()) tag = n + m; }
                void init(const std::shared_ptr<model::base> m, vk::device::handlers devices);
                void update(float dt, vk::device::handlers devices);
                
                const std::vector<VkDeviceAddress>& get_skin_gpu_address() { return skin_gpu_addresses; }

                bool has_anim() { return !raw_model->info.skeletons.empty();}
                const std::string& get_requested_model_name() { return requested_model; }
                const std::string& get_material_name() { return material_name; }

                const std::string& get_current_clip() { return anim.get_current_clip(); }
                const std::vector<std::string>& get_clip_names() { return anim.get_clip_names(); }

                void update_clip(const std::string& clip_name) { 
                    anim.play(anim.find_clip_by_name(clip_name));
                }

                uint32_t get_model_id() { return model_id; }
                uint32_t get_material_id() { return material_id; }
                const std::string& get_tag() { return tag; }

                void set_model_id(uint32_t id) { model_id = id; }
                void set_material_id(uint32_t id) { material_id = id; }
            private:
                void compute_globals(int32_t i);
                
                std::shared_ptr<model::base> raw_model = nullptr;
                glm::mat4 world{1.0f};

                std::vector<glm::vec3> translation;
                std::vector<glm::quat> rotation;
                std::vector<glm::vec3> scale;
                std::vector<glm::mat4> matrix;
                std::vector<glm::mat4> local;
                std::vector<glm::mat4> global;

                std::vector<std::vector<glm::mat4>> skin_data;
                std::vector<vk::buffer::handlers> skin_ssbo;
                std::vector<VkDeviceAddress> skin_gpu_addresses;
                    
                uint32_t model_id = 0;
                uint32_t texture_id = 0;
                uint32_t material_id = 0;
                std::string tag;

                std::string requested_model;
                std::string material_name;
                anim::base anim;
        };
    }
};
