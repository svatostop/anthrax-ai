module;
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
                inst(const std::string& n, const std::string& m) { requested_model = n; material_name = m; }
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
                    
                uint32_t raw_model_id = 0;
                std::string requested_model;
                std::string material_name;
                anim::base anim;
        };
    }
};
