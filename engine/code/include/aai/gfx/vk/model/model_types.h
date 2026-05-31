#pragma once

#include "glm/glm.hpp"

namespace model {
    namespace types {
        struct vertex {
            glm::vec4 position = glm::vec4(0);
            glm::vec3 normal = glm::vec3(0);
            glm::vec3 color = glm::vec3(0,0,0);
            glm::vec2 uv = glm::vec2(0,0);
            float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f};
            int boneID[4] = { -1, -1, -1, -1};
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
            ~node() {
                for (auto& child : children) {
                    delete child;
                }
            }
        };
    }
}

