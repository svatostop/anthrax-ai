#pragma once
#include <string>
#include "aai/gfx/vk/backend/vk_defines.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace model {
    using skin_data_array = std::vector<std::vector<glm::mat4>>;
    struct skin_groups {
        uint32_t skin_id = 0;
        model::skin_data_array d;
    };

    namespace types {
        struct vertex {
            glm::vec4 position = glm::vec4(0);
            glm::vec3 normal = glm::vec3(0);
            glm::vec3 color = glm::vec3(0,0,0);
            glm::vec2 uv = glm::vec2(0,0);
            glm::vec4 weights = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
            glm::ivec4 bone_ids = glm::ivec4(-1, -1, -1, -1);
        };
        
        struct primitive {
            uint32_t first_index;
            uint32_t index_count;
            int32_t material_index;
        };
        struct meshes {
            std::vector<primitive> primitives;
        };

        struct node {
            std::string name;
            int32_t parent = -1;
            std::vector<int32_t> children;
            meshes mesh;
            int32_t skin_id = -1;
            glm::vec3 default_translation = glm::vec3(0,0,0);
            glm::quat default_rotation = glm::quat(1, 0, 0, 0);
            glm::vec3 default_scale = glm::vec3(1);
            glm::mat4 default_matrix = glm::mat4(1.0f);
            bool use_matrix = false;
        };
        struct skeleton {
            std::string name;
            int32_t root_node_ind = -1;
            std::vector<glm::mat4> inv_matrices;
            std::vector<int32_t> bones;
        };
    }


    namespace animation {
        struct sampler {    
            enum class interpolation_t { linear, step, cubic_spline };
            interpolation_t interpolation = interpolation_t::linear;
            std::vector<float> inputs;
            std::vector<glm::vec4> outputs;
        } ;
        struct channel {
            enum class path_t { translation, rotation, scale };
            path_t path = path_t::translation;
            int32_t node_ind = -1;
            uint32_t sampler_index;
        };
        struct clip {
            std::string name;
            std::vector<sampler> samplers;
            std::vector<channel> channels;
            float start = FLT_MAX;
            float end = FLT_MIN;
        };
    }

    struct info {
        std::vector<types::node> nodes;
        std::vector<types::skeleton> skeletons;
        std::vector<animation::clip> clips;
        std::vector<std::string> clip_names;
        std::vector<int32_t> root_nodes;
        std::vector<types::vertex> vertex_buffer;
        std::vector<uint16_t> index_buffer;
    };
    
}

