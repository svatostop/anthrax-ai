#pragma once
// #include "anthraxAI/gfx/renderhelpers.h"
#include "anthraxAI/utils/defines.h"
#include "anthraxAI/gfx/vkdefines.h"
#include "anthraxAI/gfx/bufferhelper.h"

#include "anthraxAI/gameobjects/collision.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/anim.h>

namespace Gfx
{
    #define BONE_INFLUENCE 4
    
    struct MeshPushConstants {
        int texturebind = 0;
        int storagebind = 0;
        int instancebind = 0;
        int bufferbind = 0;

    int vertex_out_offset;
    int vertex_in_offset;
    int inst_index;
    int pad2;
    };

    struct MeshInfo {
        std::string Path = "";
#ifdef COMPUTE_SKINNING
        std::vector<VertexComputeSkinning> VerticesNew;
#endif
        std::vector<Vertex> Vertices;
        BufferHelper::Buffer VertexBuffer;
        
        size_t model_id = 0;

        std::vector<uint16_t> AIindices;
        std::vector<uint16_t> Indices = {
            0, 1, 3,  3, 1, 2,
            4, 5, 6, 6, 7, 4
        };

        BufferHelper::Buffer IndexBuffer;

        Keeper::Collision::AABB aabb;

        void Clean();
    };
    typedef std::unordered_map<std::string, MeshInfo> MeshMap;

    class Mesh : public Utils::Singleton<Mesh>
    {
        public:
            void CleanAll();

            void CreateMeshes();
            void CreateMesh(aiMesh* aimesh, Gfx::MeshInfo* meshinfo);
            void CreateMeshUnited(aiMesh* aimesh, Gfx::MeshInfo* meshinfo, uint32_t indexsize);

            void UpdateDummy();
            void Update(MeshInfo& mesh);
            void UpdateMesh(Gfx::MeshInfo* meshinfo);
            MeshInfo* GetMesh(const std::string& name);
        private:
            void SetVertexBoneDefaultData(Gfx::Vertex& vertex);
            MeshMap Meshes;
    };
}
