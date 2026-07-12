module;

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
    };
    struct instance_data {
        glm::mat4 model;
    };
    template <typename T>
    struct gpu_data {
        T raw_data;
        vk::buffer::handlers data;
    };
};
