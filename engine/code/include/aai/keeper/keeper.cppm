export module aai.keeper;
import aai.keeper.camera;
import aai.keeper.entity;
import std;
import glm;
export {
    namespace keeper {
        typedef int entity_id;
        typedef std::map<entity_id, std::shared_ptr<entity>> entity_map;
        class base {
            public:
                template<typename T>
                void create() { std::shared_ptr<T> e = std::make_shared<T>(); entities[e->get_id()] = e; }
                entity_id get_last_id() { return std::prev(entities.end())->first; }

                std::shared_ptr<entity> get(const entity_id id) { return entities[id]; }

                void update() {
                    for (entity_map::iterator it = entities.begin(); it != entities.end(); ++it) {
                        it->second->update();
                    }
                }
            private:
                entity_map entities;
        };
    }
};
