#include "anthraxAI/gfx/model.h"
#include "anthraxAI/core/scene.h"
#include "anthraxAI/gfx/vkbase.h"
#include <cstdint>
#include <cstring>

void Gfx::Model::SetVertexBoneData(Gfx::Vertex& vert, int id, float weight)
{
    for (int i = 0; i < BONE_INFLUENCE; i++) {
           if (vert.boneID[i] < 0) {
               vert.boneID[i] = id;
               vert.weights[i] = weight;
               break;
           }
    }
}

void Gfx::Model::ProcessBones(std::vector<Vertex>& vert, const std::string& path, aiMesh* aimesh, uint32_t vertsize)
{
   for(unsigned int j = 0; j < aimesh->mNumBones; j++) {

        int boneID = -1;
        ModelInfo& model = Models[path];
        std::string bonename = aimesh->mBones[j]->mName.C_Str();
        if (model.Bones.BoneMap.find(bonename) == model.Bones.BoneMap.end()) {
            Gfx::Bone bone;
            model.Bones.Info.push_back(bone);
            boneID = BoneCounter;
            BoneCounter++;
        }
        else {
            boneID = model.Bones.BoneMap[bonename];
        }

        model.Bones.BoneMap[bonename] = boneID;
        model.Bones.Info[boneID] = mat2glm(aimesh->mBones[j]->mOffsetMatrix);
        ASSERT(boneID == -1, "bone loading error\n");
        aiVertexWeight* weights = aimesh->mBones[j]->mWeights;
        int numweights = aimesh->mBones[j]->mNumWeights;
        int vertexID = 0;
        for (int weightind = 0; weightind < numweights; ++weightind) {
            vertexID = weights[weightind].mVertexId + vertsize;
            float weight = weights[weightind].mWeight;
            ASSERT(!(vertexID <= vert.size()), "vertex weight loading error\n");
            SetVertexBoneData(vert[vertexID], boneID, weight);
        }
    }
}
void Gfx::Model::ProcessNode2(const std::string& path, aiNode *node, const aiScene *scene)
{
    Models[path].MeshBase.resize(scene->mNumMeshes);
    Models[path].texturename = path;
    int meshsize = scene->mNumMeshes;

    Models[path].Meshes.reserve(meshsize);
    Gfx::MeshInfo* meshinfo = new Gfx::MeshInfo;
    uint32_t ind = 0;
    uint32_t boneind = 0;
    for (int i = 0; i < meshsize; i++) {

        aiMesh* aimesh = scene->mMeshes[i];
    
        ind  = meshinfo->Vertices.empty() ? 0 : meshinfo->Vertices.size();
        Gfx::Mesh::GetInstance()->CreateMeshUnited(aimesh, meshinfo,ind);

        Models[path].MeshBase[i] = TotalVertex;
        TotalVertex += aimesh->mNumVertices;
        Models[path].Bones.Vertext2Bone.resize(TotalVertex);

        if (aimesh->HasBones()) {
            ProcessBones(meshinfo->Vertices, path, aimesh, boneind);
            boneind = meshinfo->Vertices.size();
        }
    }
    Gfx::Mesh::GetInstance()->UpdateMesh(meshinfo);
    Models[path].Meshes.push_back(meshinfo);
}

void Gfx::Model::ProcessNode(const std::string& path, aiNode *node, const aiScene *scene)
{
    Models[path].MeshBase.resize(scene->mNumMeshes);
    Models[path].texturename = path;
    int meshsize = scene->mNumMeshes;
    Models[path].Meshes.reserve(meshsize);
    for (int i = 0; i < meshsize; i++) {

        aiMesh* aimesh = scene->mMeshes[i];

        Gfx::MeshInfo* meshinfo = new Gfx::MeshInfo;
        Gfx::Mesh::GetInstance()->CreateMesh(aimesh, meshinfo);

        Models[path].MeshBase[i] = TotalVertex;
        TotalVertex += aimesh->mNumVertices;
        Models[path].Bones.Vertext2Bone.resize(TotalVertex);

        if (aimesh->HasBones()) {
            ProcessBones(meshinfo->Vertices, path, aimesh);
        }
        Gfx::Mesh::GetInstance()->UpdateMesh(meshinfo);
        Models[path].Meshes.push_back(meshinfo);
    }
}

void Gfx::Model::CleanAll()
{
    total_vertecies = 0;
    for (auto& model : Models) {
        for (auto& mesh : model.second.Meshes) {
            mesh->Clean();
            delete mesh;
        }
        model.second.Meshes.clear();
    }
    Models.clear();
}

void Gfx::Model::LoadModels()
{
    std::string path = "./models/";
    Core::Scene* scene = Core::Scene::GetInstance();
    for (auto& it : scene->GetGameObjects()->GetObjects()) {
        for (Keeper::Objects* info : it.second) {
            if (info->GetModelName().empty()) continue;
            if (GetModel(info->GetModelName())) {
                continue; ;
            }
            LoadModel(path + info->GetModelName());
        }
    }
    //  int s = 0;
    // for (auto& v : Models) {
    //     for (MeshInfo* i : v.second.Meshes) {
    //         s += i->Vertices.size();
    //     }
    // }
    if (ModelNames.empty()) {
        ModelNames.reserve(100);
        std::vector<std::string> names;
        names.reserve(20);
        for (const auto& name : std::filesystem::directory_iterator(path)) {
            std::string str = name.path().string();
            std::string basename = str.substr(str.find_last_of("/\\") + 1);
            bool exists = basename.find(".obj") != std::string::npos || basename.find(".fbx") != std::string::npos || basename.find(".dae") != std::string::npos;
            if (exists) {
                ModelNames.push_back(basename);
            }
        }
    }

}

void Gfx::Model::LoadModel(const std::string& path)
{
    Assimp::Importer importer;
    const aiScene* scene = nullptr;

    scene = importer.ReadFile(path, IMPORT_PROPS);
    ASSERT(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode, (importer.GetErrorString()));

    TotalVertex = 0;
    BoneCounter = 0;
    // if (scene->HasAnimations()) {
    //     ProcessNode(path, scene->mRootNode, scene);
    // }
    // else {
        ProcessNode2(path, scene->mRootNode, scene);
    // }
    
    total_vertecies += TotalVertex;
    importer.FreeScene();
}

Gfx::ModelInfo* Gfx::Model::GetModel(const std::string& path)
{
	auto it = Models.find("./models/" + path);
	if (it == Models.end()) {
		return nullptr;
	}
	else {
		return &(*it).second;
	}
}
