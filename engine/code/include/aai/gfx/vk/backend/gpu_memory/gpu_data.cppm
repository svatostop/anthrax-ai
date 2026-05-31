module;

export module aai.gfx.vk.gpu_data;
export import aai.gfx.vk.buffer;
import glm;
export {
    struct camera_data {
        glm::vec4 viewport;
        glm::vec4 test_color;
    };
    template <typename T>
    struct gpu_data {
        T raw_data;
        vk::buffer::handlers data;
    };
};
