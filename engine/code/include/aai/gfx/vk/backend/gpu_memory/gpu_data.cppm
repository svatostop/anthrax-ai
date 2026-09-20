module;
#include <strings.h>

export module aai.gfx.vk.gpu_data;
export import aai.gfx.vk.buffer;
import glm;
export {
    enum class gpu_data_type {
        CAMERA =0,
        INSTANCE
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
    template <typename T>
    struct gpu_data {
        T raw_data;
        T* mapped_data = nullptr;
        size_t buffer_size = 0;
        size_t submitted_size = 0;
        vk::buffer::handlers data;
    };
};
