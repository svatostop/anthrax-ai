#pragma once

#include "anthraxAI/utils/defines.h"
#include "anthraxAI/gfx/vkdefines.h"
#include "anthraxAI/gfx/vkmesh.h"
#include "anthraxAI/core/deletor.h"
#include "glm/fwd.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/anim.h>
#include <map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#define IMPORT_PROPS (aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeGraph)
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

namespace Gfx
{
    struct VertexBoneData {
        int BoneId[BONE_INFLUENCE]  ={0};
	    float Weight[BONE_INFLUENCE]  ={0.0f};

	    void Add(int id, float w) {
		  for (int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(BoneId); i++) {
		  	if (Weight[i] == 0.0f) {
		  		BoneId[i] = id;
		  		Weight[i] = w;
		  		return;
		  	}
		  }
	    }
    };
    struct Bone {
        glm::mat4 Offset;
	    glm::mat4 FinTransform = glm::mat4(1);
	    Bone() {};
	    Bone(const glm::mat4& offs) {
		  Offset = offs;
		  FinTransform = glm::mat4(0.0);
	    }
    };
    struct BoneInfo {
        std::vector<VertexBoneData> Vertext2Bone;
        std::unordered_map<std::string, int> BoneMap;
        std::vector<Bone> Info;
        std::vector<glm::mat4> FinTransform;
    };
    struct ModelInfo {
        std::string texturename;
        std::vector<MeshInfo*> Meshes;
        std::vector<int> MeshBase;

        BoneInfo Bones;
        
    };
    typedef std::unordered_map<std::string, ModelInfo> ModelsMap;

    class Model : public Utils::Singleton<Model>
    {
        public:
            Model() {}
            void LoadModels();
            void LoadModel(const std::string& path);
            ModelInfo* GetModel(const std::string& path);
            const std::vector<std::string>& GetModelNames() const { return ModelNames; }         
            
            const ModelsMap& GetModels() const { return Models; }
            ModelsMap& GetNonConstModels()  { return Models; }
            void CleanAll();
            int GetVerteciesSize() const { return total_vertecies; } 
        private:
            void ProcessBones(std::vector<Vertex>& vert, const std::string& path, aiMesh* aimesh, uint32_t vertsize = 0);
            void ProcessNode(const std::string& path, aiNode *node, const aiScene *scene);
            void ProcessNode2(const std::string& path, aiNode *node, const aiScene *scene);

            void SetVertexBoneData(Gfx::Vertex& vert, int id, float weight);
            
            std::vector<std::string> ModelNames;
            ModelsMap Models;
            int TotalVertex = 0;
            int BoneCounter = 0;

            int total_vertecies = 0;
    };

    static inline glm::mat4 mat2glm(aiMatrix4x4 from)
    {
    	glm::mat4 to;
    	//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
    	to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    	to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    	to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    	to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    	return to;
    }

    static inline glm::mat3 mat2glm(aiMatrix3x3 from)
    {
    	glm::mat3 to;
    	//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
    	to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3;
    	to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3;
    	to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3;
    	return to;
    }

    static inline glm::quat quat2glm(const aiQuaternion& pOrientation)
    	{
    		return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
    	}

    static inline glm::vec3 vec2glm(aiVector3D vec)
    {
    	glm::vec3 v;
    	v.x = vec.x;
    	v.y = vec.y;
    	v.z = vec.z;
    	return v;
    }

}
