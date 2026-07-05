module;
#include "aai/io/win_defines.h"

export module aai.gfx;

export import aai.keeper.camera;
export import aai.gfx.vk;
export import aai.gfx.assets;
export import aai.gfx.vk.rt;
export import aai.gfx.materials;
import std;
import glm;
export {
    namespace gfx {
        class base {
            public:
                void init(GLFWwindow* glfw_win, Display* di, Window w);
                
                void run();
                void populate();

                void set_camera(std::shared_ptr<keeper::camera> c) { cam = c; cam->set_position(glm::vec3(-2.0f, 0.0f, -10.0f)); }

                uint32_t create_texture(const char* path);
                uint32_t create_model(const char* path);

                void clean() { vk.wait_timeline(); clean_resources(); }
            private:
                void clean_resources();
                vk::base vk;
                assets::base<rt::render_target> asset_mng;
                assets::base<model::base> model_mng;
                mat::materials material_pallet;

                glm::ivec2 window_size;
                std::shared_ptr<keeper::camera> cam;
        };
    }
};
