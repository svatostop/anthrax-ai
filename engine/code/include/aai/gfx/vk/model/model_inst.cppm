module;
#include "aai/gfx/vk/model/model_types.h"

export module aai.gfx.vk.model.inst;
import aai.gfx.vk.model.anim;
export import aai.gfx.vk.model;
export {
    namespace model {
        class inst {
            public:
                void init(const model::base* m);
                void update(float dt);

                std::vector<glm::vec3>& get_translation();
                std::vector<glm::quat>& get_rotation();
                std::vector<glm::vec3>& get_scale();
            private:
                void compute_globals(int32_t i);
                
                const model::base* asset = nullptr;
                glm::mat4 world{1.0f};

                std::vector<glm::vec3> translation;
                std::vector<glm::quat> rotation;
                std::vector<glm::vec3> scale;
                std::vector<glm::mat4> matrix;
                std::vector<glm::mat4> local;
                std::vector<glm::mat4> global;

                std::vector<std::vector<glm::mat4>> skin_data;

                anim::base anim;
        };
    }
};
