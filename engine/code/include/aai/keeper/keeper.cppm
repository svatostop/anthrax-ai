export module aai.keeper;
import aai.keeper.camera;
import std;
import glm;
export {
    namespace keeper {
        class base {
            public:
                void init(glm::vec3 camera_pos);

                std::shared_ptr<camera> get_camera(keeper::camera::type t) { return cam; } 
            private:
                std::shared_ptr<camera> cam = nullptr;
        };
    }
};
