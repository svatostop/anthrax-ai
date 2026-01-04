#include "anthraxAI/gfx/vkmesh.h"
#include "anthraxAI/core/windowmanager.h"
#include "anthraxAI/gfx/vkdevice.h"
#include "anthraxAI/gfx/vkrenderer.h"
#include "imgui.h"
#include <cstdint>
#include <cstdio>

Gfx::MeshInfo* Gfx::Mesh::GetMesh(const std::string& name)
{
    MeshMap::iterator it = Meshes.find(name);
	if (it == Meshes.end()) {
		return nullptr;
	}
	else {
		return &(*it).second;
	}
}

void Gfx::Mesh::Update(MeshInfo& mesh)
{
    VkBufferUsageFlags flags[2] = {VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT};
#ifdef COMPUTE_SKINNING
    if (!mesh.VerticesNew.empty()) {
	    BufferHelper::CreateBuffer(mesh.VertexBuffer, flags, sizeof(mesh.VerticesNew[0]) * mesh.VerticesNew.size(), mesh.VerticesNew.data());
    }
    else
#endif
    {
	    BufferHelper::CreateBuffer(mesh.VertexBuffer, flags, sizeof(mesh.Vertices[0]) * mesh.Vertices.size(), mesh.Vertices.data());
    }
	VkBufferUsageFlags flags2[2] = {VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT};
	if (mesh.AIindices.empty()) {
		BufferHelper::CreateBuffer(mesh.IndexBuffer, flags2, sizeof(mesh.Indices[0]) * mesh.Indices.size(), mesh.Indices.data());
	}
	else {
		BufferHelper::CreateBuffer(mesh.IndexBuffer, flags2, sizeof(mesh.AIindices[0]) * mesh.AIindices.size(), mesh.AIindices.data());
	}

    VkDebugUtilsObjectNameInfoEXT info;
	info.pNext = nullptr;
	info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	info.objectHandle = reinterpret_cast<uint64_t>(mesh.IndexBuffer.DeviceMemory);
	info.objectType = VK_OBJECT_TYPE_DEVICE_MEMORY;;
	info.pObjectName = std::string("mesh index buffer" + mesh.Path).c_str();
	Gfx::Vulkan::GetInstance()->SetDebugName(info);

	info.pNext = nullptr;
	info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	info.objectHandle = reinterpret_cast<uint64_t>(mesh.VertexBuffer.DeviceMemory);
	info.objectType = VK_OBJECT_TYPE_DEVICE_MEMORY;;
	info.pObjectName = std::string("mesh vertex buffer" + mesh.Path).c_str();
	Gfx::Vulkan::GetInstance()->SetDebugName(info);

}

void Gfx::Mesh::SetVertexBoneDefaultData(Gfx::Vertex& vertex)
{
  for (int i = 0; i < BONE_INFLUENCE; i++)
    {
        vertex.boneID[i] = -1;
        vertex.weights[i] = 0.0f;
    }
}
void Gfx::Mesh::CreateMesh(aiMesh* aimesh, Gfx::MeshInfo* meshinfo)
{
   // Gfx::MeshInfo meshinfo;
    meshinfo->Vertices.reserve(aimesh->mNumVertices);
    for(int i = 0; i < aimesh->mNumVertices; i++) {
        Gfx::Vertex vertex;

        SetVertexBoneDefaultData(vertex);

		glm::vec3 data;
		data.x = aimesh->mVertices[i].x;
		data.y = aimesh->mVertices[i].y;
		data.z = aimesh->mVertices[i].z;
		vertex.position.x = data.x;
		vertex.position.y = data.y;
		vertex.position.z = data.z;

		data.x = aimesh->mNormals[i].x;
		data.y = aimesh->mNormals[i].y;
		data.z = aimesh->mNormals[i].z;
		vertex.normal = data;
		if(aimesh->mTextureCoords[0]) {
			glm::vec2 vec;
			vec.x = aimesh->mTextureCoords[0][i].x;
			vec.y = aimesh->mTextureCoords[0][i].y;
			vertex.uv = vec;
		}
		else {
			vertex.uv = glm::vec2(0);
		}
		vertex.color = {1.0f, 1.0f, 1.0f};
        vertex.color = vertex.normal;

		meshinfo->Vertices.push_back(vertex);
    }

    meshinfo->AIindices.reserve(aimesh->mNumFaces);
	for(unsigned int i = 0; i < aimesh->mNumFaces; i++) {
		aiFace face = aimesh->mFaces[i];
		for(unsigned int j = 0; j < face.mNumIndices; j++) {
			meshinfo->AIindices.push_back(face.mIndices[j]);
		}
	}
    
     meshinfo->aabb = Keeper::Collision::Create(meshinfo->Vertices);
}

void Gfx::Mesh::CreateMeshUnited(aiMesh* aimesh, Gfx::MeshInfo* meshinfo, uint32_t indexsize)
{
   // Gfx::MeshInfo meshinfo;
  //  meshinfo->Vertices.reserve(aimesh->mNumVertices);
    for(int i = 0; i < aimesh->mNumVertices; i++) {
        Gfx::Vertex vertex;

        SetVertexBoneDefaultData(vertex);

		glm::vec3 data;
		data.x = aimesh->mVertices[i].x;
		data.y = aimesh->mVertices[i].y;
		data.z = aimesh->mVertices[i].z;
		vertex.position.x = data.x;
		vertex.position.y = data.y;
		vertex.position.z = data.z;

		data.x = aimesh->mNormals[i].x;
		data.y = aimesh->mNormals[i].y;
		data.z = aimesh->mNormals[i].z;
		vertex.normal = data;
		if(aimesh->mTextureCoords[0]) {
			glm::vec2 vec;
			vec.x = aimesh->mTextureCoords[0][i].x;
			vec.y = aimesh->mTextureCoords[0][i].y;
			vertex.uv = vec;
		}
		else {
			vertex.uv = glm::vec2(0);
		}
		vertex.color = {1.0f, 1.0f, 1.0f};
        vertex.color = vertex.normal;

		meshinfo->Vertices.push_back(vertex);
    }

   // meshinfo->AIindices.reserve(aimesh->mNumFaces);
        
	for(unsigned int i = 0; i < aimesh->mNumFaces; i++) {
        aiFace face = aimesh->mFaces[i];
		for(unsigned int j = 0; j < face.mNumIndices; j++) {
			meshinfo->AIindices.push_back(indexsize + face.mIndices[j]);
		}
	}
    
     meshinfo->aabb = Keeper::Collision::Create(meshinfo->Vertices);
}

void Gfx::Mesh::UpdateMesh(Gfx::MeshInfo* meshinfo)
{
    Update(*meshinfo);
}

void Gfx::MeshInfo::Clean()
{
    vkDestroyBuffer(Gfx::Device::GetInstance()->GetDevice(), VertexBuffer.Buffer, nullptr);
    vkFreeMemory(Gfx::Device::GetInstance()->GetDevice(), VertexBuffer.DeviceMemory, nullptr);
    vkDestroyBuffer(Gfx::Device::GetInstance()->GetDevice(), IndexBuffer.Buffer, nullptr);
    vkFreeMemory(Gfx::Device::GetInstance()->GetDevice(), IndexBuffer.DeviceMemory, nullptr);
}

void Gfx::Mesh::CleanAll()
{
    for (auto& it : Meshes) {
        Gfx::MeshInfo& mesh = it.second;
        mesh.Clean();
    }
    Meshes.clear();
}

void Gfx::Mesh::UpdateDummy()
{
    MeshInfo* tmp = GetMesh("dummy");
    tmp->Clean();

    Gfx::MeshInfo mesh;
#ifndef COMPUTE_SKINNING
    mesh.Vertices.resize(4);
#else
    mesh.VerticesNew.resize(4);
#endif
    Vector2<int> res = Core::WindowManager::GetInstance()->GetScreenResolution();
    mesh.Path = "dummy";
#ifndef COMPUTE_SKINNING
    mesh.Vertices[0].position = {0, 0, 0.0f, 1.0f};
    mesh.Vertices[1].position = {0, 0 + res.y, 0.0f, 1.0f};
    mesh.Vertices[2].position = {0 + res.x, 0 + res.y, 0.0f, 1.0f};
    mesh.Vertices[3].position ={0 + res.x, 0, 0.0f, 1.0f};

    mesh.Vertices[0].color = { 0.f, 1.f, 0.0f};
    mesh.Vertices[1].color = { 0.f, 1.f, 0.0f};
    mesh.Vertices[2].color = { 0.f, 1.f, 0.0f};
    mesh.Vertices[3].color = { 1.f, 0.f, 0.0f};

    mesh.Vertices[1].uv = {0.0f, 0.0};
    mesh.Vertices[0].uv = {0.0f, 1.0};
    mesh.Vertices[3].uv = {1.0f, 1.0};
    mesh.Vertices[2].uv = {1.0f, 0.0};

#else
    mesh.VerticesNew[0].position = {0, 0, 0.0f, 1.0f};
    mesh.VerticesNew[1].position = {0, 0 + res.y, 0.0f, 1.0f};
    mesh.VerticesNew[2].position = {0 + res.x, 0 + res.y, 0.0f, 1.0f};
    mesh.VerticesNew[3].position ={0 + res.x, 0, 0.0f, 1.0f};

    mesh.VerticesNew[0].color = { 0.f, 1.f, 0.0f, 1.0f};
    mesh.VerticesNew[1].color = { 0.f, 1.f, 0.0f, 1.0f};
    mesh.VerticesNew[2].color = { 0.f, 1.f, 0.0f, 1.0f};
    mesh.VerticesNew[3].color = { 1.f, 0.f, 0.0f, 1.0f};

    mesh.VerticesNew[1].uv = {0.0f, 0.0, 0.0f, 1.0f};
    mesh.VerticesNew[0].uv = {0.0f, 1.0, 0.0f, 1.0f};
    mesh.VerticesNew[3].uv = {1.0f, 1.0, 0.0f, 1.0f};
    mesh.VerticesNew[2].uv = {1.0f, 0.0, 0.0f, 1.0f};
#endif
    Update(mesh);
    Meshes["dummy"] = mesh;

}

void Gfx::Mesh::CreateMeshes()
{
    Gfx::TexturesMap texturemap = Gfx::Renderer::GetInstance()->GetTextureMap();

    Gfx::MeshInfo mesh;
#ifndef COMPUTE_SKINNING
    mesh.Vertices.resize(4);
#else
    mesh.VerticesNew.resize(4);
#endif
    for (auto it : texturemap) {
        if (it.first == "dummy") continue;
        mesh.Path = it.first;
#ifndef COMPUTE_SKINNING
        mesh.Vertices[0].position = {0, 0, 0.0f, 1.0f};
        mesh.Vertices[1].position = {0, 0 + it.second.GetSize().y, 0.0f, 1.0f};
        mesh.Vertices[2].position = {0 + it.second.GetSize().x, 0 + it.second.GetSize().y, 0.0f, 1.0f};
        mesh.Vertices[3].position ={0 + it.second.GetSize().x, 0, 0.0f, 1.0f};

        mesh.Vertices[0].color = { 0.f, 1.f, 0.0f };
        mesh.Vertices[1].color = { 0.f, 1.f, 0.0f };
        mesh.Vertices[2].color = { 0.f, 1.f, 0.0f };
        mesh.Vertices[3].color = { 1.f, 0.f, 0.0f };

        mesh.Vertices[0].uv = {0.0f, 0.0f};
        mesh.Vertices[1].uv = {0.0f, 1.0f};
        mesh.Vertices[2].uv = {1.0f, 1.0f};
        mesh.Vertices[3].uv = {1.0f, 0.0f};
#else
        mesh.VerticesNew[0].position = {0, 0, 0.0f, 1.0f};
        mesh.VerticesNew[1].position = {0, 0 + it.second.GetSize().y, 0.0f, 1.0f};
        mesh.VerticesNew[2].position = {0 + it.second.GetSize().x, 0 + it.second.GetSize().y, 0.0f, 1.0f};
        mesh.VerticesNew[3].position ={0 + it.second.GetSize().x, 0, 0.0f, 1.0f};
    
        mesh.VerticesNew[0].color = { 0.f, 1.f, 0.0f, 1.0f};
        mesh.VerticesNew[1].color = { 0.f, 1.f, 0.0f, 1.0f};
        mesh.VerticesNew[2].color = { 0.f, 1.f, 0.0f, 1.0f};
        mesh.VerticesNew[3].color = { 1.f, 0.f, 0.0f, 1.0f};
    
        mesh.VerticesNew[1].uv = {0.0f, 0.0, 0.0f, 1.0f};
        mesh.VerticesNew[0].uv = {0.0f, 1.0, 0.0f, 1.0f};
        mesh.VerticesNew[3].uv = {1.0f, 1.0, 0.0f, 1.0f};
        mesh.VerticesNew[2].uv = {1.0f, 0.0, 0.0f, 1.0f};
#endif
        Update(mesh);
        Meshes[it.first] = mesh;
    }

    Vector2<int> res = Core::WindowManager::GetInstance()->GetScreenResolution();//, 1080 };
    mesh.Path = "dummy";
#ifndef COMPUTE_SKINNING
    mesh.Vertices[0].position = {0, 0, 0.0f, 1.0f};
    mesh.Vertices[1].position = {0, 0 + res.y, 0.0f, 1.0f};
    mesh.Vertices[2].position = {0 + res.x, 0 + res.y, 0.0f, 1.0f};
    mesh.Vertices[3].position ={0 + res.x, 0, 0.0f, 1.0f};

    mesh.Vertices[0].color = { 0.f, 1.f, 0.0f };
    mesh.Vertices[1].color = { 0.f, 1.f, 0.0f };
    mesh.Vertices[2].color = { 0.f, 1.f, 0.0f };
    mesh.Vertices[3].color = { 1.f, 0.f, 0.0f };

    mesh.Vertices[1].uv = {0.0f, 0.0f};
    mesh.Vertices[0].uv = {0.0f, 1.0f};
    mesh.Vertices[3].uv = {1.0f, 1.0f};
    mesh.Vertices[2].uv = {1.0f, 0.0f};
#else
    mesh.VerticesNew[0].position = {0, 0, 0.0f, 1.0f};
    mesh.VerticesNew[1].position = {0, 0 + res.y, 0.0f, 1.0f};
    mesh.VerticesNew[2].position = {0 + res.x, 0 + res.y, 0.0f, 1.0f};
    mesh.VerticesNew[3].position ={0 + res.x, 0, 0.0f, 1.0f};

    mesh.VerticesNew[0].color = { 0.f, 1.f, 0.0f, 1.0f};
    mesh.VerticesNew[1].color = { 0.f, 1.f, 0.0f, 1.0f};
    mesh.VerticesNew[2].color = { 0.f, 1.f, 0.0f, 1.0f};
    mesh.VerticesNew[3].color = { 1.f, 0.f, 0.0f, 1.0f};

    mesh.VerticesNew[1].uv = {0.0f, 0.0, 0.0f, 1.0f};
    mesh.VerticesNew[0].uv = {0.0f, 1.0, 0.0f, 1.0f};
    mesh.VerticesNew[3].uv = {1.0f, 1.0, 0.0f, 1.0f};
    mesh.VerticesNew[2].uv = {1.0f, 0.0, 0.0f, 1.0f};
#endif
    Update(mesh);
    Meshes["dummy"] = mesh;

}
