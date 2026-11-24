module;
#include <cstdint>
#include <strings.h>

export module aai.gfx.vk.gpu_data;
export import aai.gfx.vk.buffer;
import glm;
export {
    enum class gpu_data_type {
        CAMERA =0,
        INSTANCE,
        INSTANCED_RECT,
    };
    struct camera_data {
        glm::vec4 viewport;
        glm::vec4 test_color;

        glm::mat4 view;
        glm::mat4 proj;
        glm::mat4 non_reverse_proj;
    };
    struct instance_data {
        glm::mat4 model;
    };

    struct instanced_rect {
        glm::vec2 position;
        glm::vec2 size;
        glm::vec4 uv;
        glm::vec4 color;

        uint32_t texture_id = 0;
        uint32_t has_outline = 0;
    };
    
    namespace gfx::dbg {
        struct point {
            glm::vec4 p;
            glm::mat4 model;

            point() {}
            point(const glm::vec3& d) : p(glm::vec4(d.x, d.y, d.z, 1)) {}
            point(const glm::vec4& d) : p(d) {}
            point(const glm::vec3& d, const glm::mat4& m) : p(glm::vec4(d.x, d.y, d.z, 1)), model(m) {}
            point(const glm::vec4& d, const glm::mat4& m) : p(d), model(m) {}
        };
    }

    template <typename T>
    struct gpu_data {
        T raw_data;
        T* mapped_data = nullptr;
        size_t buffer_size = 0;
        bool submitted = false;
        vk::buffer::handlers data;
    };
};
