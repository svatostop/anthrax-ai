#pragma once
#include <string>
#include "aai/gfx/vk/backend/vk_defines.h"

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace model {
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
            node* parent;
            std::vector<node*> children;
            meshes mesh;
            glm::mat4 matrix;
            std::string name;
            bool visible = true;
            glm::vec3 translation{};
            glm::vec3 scale{1.0f};
            glm::quat rotation{};
            int32_t   skin_id = -1;
            int32_t   index = -1;
            glm::mat4 get_local_matrix() {
	            return glm::translate(glm::mat4(1.0f), translation) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale) * matrix;
            }
            ~node() {
                for (auto& child : children) {
                    delete child;
                }
            }
        };
        struct skin {
            std::string name;
            node* skeleton_root = nullptr;
            std::vector<glm::mat4> inv_matrices;
            std::vector<node*> bones;
        };
    }

    namespace animation {
        struct sampler {
            std::string interpolation;
            std::vector<float> inputs;
            std::vector<glm::vec4> outputs;
        } ;
        struct channel {
            std::string path;
            types::node* n;
            uint32_t sampler_index;
        };
        struct base {
            std::string name;
            std::vector<sampler> samplers;
            std::vector<channel> channels;
            float start = FLT_MAX;
            float end = FLT_MIN;
            float current_time = 0.0f;
        };
    }

}

