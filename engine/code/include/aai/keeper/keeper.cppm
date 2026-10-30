module;
#include "glm/ext/matrix_transform.hpp"

export module aai.keeper;
import aai.keeper.types;
import aai.keeper.camera;
import aai.keeper.entity;
import aai.gfx.vk.model.inst;
import aai.utils.threads.job_sys;
import aai.editor;
import aai.gfx.debug;
import std;
import glm;
export {
    namespace keeper {
        typedef int entity_id;
        typedef std::map<entity_id, std::shared_ptr<entity>> entity_map;
        class base {
            public:
                base() {
                    instances.reserve(3);
                    model::inst& cube = instances.emplace_back("./models/cube.glb", "material_model");
                    model::inst& fox = instances.emplace_back("./models/fox.glb", "material_model_anim", glm::translate(glm::mat4(1), glm::vec3(0.0, 1.0, 1.0)));
                    model::inst& robot = instances.emplace_back("./models/robot.glb", "material_model_anim", glm::translate(glm::mat4(1), glm::vec3(2.0, 2.0, 1.0)));
                }

                template<typename T>
                entity_id create() { std::shared_ptr<T> e = std::make_shared<T>(); entities[e->get_id()] = e; return e->get_id(); }

                std::shared_ptr<entity> get(const entity_id id) { return entities[id]; }

                // todo move to cpp
                void update(float delta_ms) {
                    for (entity_map::iterator it = entities.begin(); it != entities.end(); ++it) {
                        it->second->update(delta_ms);
                    }
                    for (std::vector<model::inst>::iterator it = instances.begin(); it != instances.end(); ++it) {
                        back_datas& data = back_data.emplace_back();
                        utils::job_sys::execute([this, delta_ms, it = std::move(it), &data]() {
                            it->update(delta_ms * 0.001f);
                            data.model_id = it->get_model_id();
                            data.material_id = it->get_material_id();
                            data.tag = it->get_tag();
                            data.skin_group = it->get_skin_group();
                            data.model_matrix = it->get_model_matrix();
                        });
                    }
                    utils::job_sys::wait();
                }
                void watch(model::inst& inst, aai::editor& editor) {
                    editor.watch(inst.get_clip_names(), [&](const std::string& s) {
                        if (s != inst.get_current_clip())
                            inst.update_clip(s);
                    });
                }

                std::deque<back_datas>& get_back_data() { return back_data; }

                model::inst& get_test(std::uint32_t id) { return instances[id]; }
            private:
                std::vector<model::inst> instances;
                std::deque<back_datas> back_data;

                entity_map entities;
        };
    }
};
