#pragma once

#include "anthraxAI/utils/defines.h"

#include "anthraxAI/gameobjects/objects/npc.h"
#include "anthraxAI/gameobjects/objects/camera.h"

namespace Gfx {
struct VertexComputeSkinning {
        glm::vec4 position = glm::vec4(0);
        glm::vec4 normal = glm::vec4(0);
        glm::vec4 color = glm::vec4(0);
        glm::vec4 uv = glm::vec4(0);
    };
struct VertexComputeSkinningIn {
        glm::vec4 position = glm::vec4(0);
        glm::vec4 normal = glm::vec4(0);
        glm::vec4 color = glm::vec4(0);
        glm::vec4 uv = glm::vec4(0);
        glm::vec4 vweight;
        glm::ivec4 vboneid;
   
        glm::ivec4 datas;

    };
 struct Vertex {
        glm::vec4 position = glm::vec4(0);
        glm::vec3 normal = glm::vec3(0);
        glm::vec3 color = glm::vec3(0,0,0);
        glm::vec2 uv = glm::vec2(0,0);
        float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f};
        int boneID[4] = { -1, -1, -1, -1};
    };
    struct ComputeVertex {
        glm::vec2 position;
        glm::vec2 velocity;
        glm::vec4 color;
    };

}

namespace Keeper
{ 
    namespace Collision
    {
        struct AABB {
            glm::vec3 min = {};
            glm::vec3 max = {};
        };
    
        bool Cull(const glm::mat4& vp, const Keeper::Objects* data);
        Keeper::Collision::AABB  Create(const std::vector<Gfx::Vertex>& verts);
    }    
}

