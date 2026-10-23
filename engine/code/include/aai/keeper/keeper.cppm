export module aai.keeper;
import aai.keeper.camera;
import aai.keeper.entity;
import aai.gfx.vk.model.inst;
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
                    model::inst& fox = instances.emplace_back("./models/fox.glb", "material_model_anim");
                    model::inst& robot = instances.emplace_back("./models/robot.glb", "material_model_anim");
                }

                template<typename T>
                entity_id create() { std::shared_ptr<T> e = std::make_shared<T>(); entities[e->get_id()] = e; return e->get_id(); }

                std::shared_ptr<entity> get(const entity_id id) { return entities[id]; }

                void update(float delta_ms) {
                    for (entity_map::iterator it = entities.begin(); it != entities.end(); ++it) {
                        it->second->update(delta_ms);
                    }
                }

                const model::inst& get_test(std::uint32_t id) { return instances[id]; }
            private:
                std::vector<model::inst> instances;

                entity_map entities;
        };
    }
};
