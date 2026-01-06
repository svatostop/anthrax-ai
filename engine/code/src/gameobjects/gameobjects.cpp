#include "anthraxAI/gameobjects/gameobjects.h"
#include "anthraxAI/gameobjects/collision.h"
#include "anthraxAI/gameobjects/objects/camera.h"
#include "anthraxAI/gameobjects/objects/light.h"
#include "anthraxAI/gameobjects/objects/sprite.h"
#include "anthraxAI/gameobjects/objects/npc.h"
#include "anthraxAI/gameobjects/objects/gizmo.h"
#include "anthraxAI/core/windowmanager.h"
#include "anthraxAI/utils/defines.h"
#include "anthraxAI/utils/mathdefines.h"
#include "glm/common.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <iterator>
#include <string>
#include <vector>
Keeper::Base::~Base()
{
  for (auto& it : ObjectsList) {
    for (auto* obj : it.second) {
      if (obj) {
        delete obj;
      }
    }
  }
  ObjectsList.clear();
}

void Keeper::Base::CleanIfNot(Keeper::Type type, bool resetID)
{
    for (auto it = ObjectsList.begin(); it != ObjectsList.end(); ) {
        if (it->first != type) {
            for (auto* obj : it->second) {
                if (resetID) {
                    obj->ResetCounterID();
                }
                if (obj) {
                    delete obj;
                }
            }
            it->second.clear();
            ObjectsList.erase(it++);
        }
        else {
            ++it;
        }
    }
}

const Keeper::Objects* Keeper::Base::GetObject(Keeper::Type type, int id) const
{
    std::vector<Keeper::Objects*> vec = ObjectsList.at(type);

    auto it = std::find_if(vec.begin(), vec.end(), [id](Keeper::Objects* o) { return o->GetID() == id; });

    return *it;
}

Keeper::Objects* Keeper::Base::GetNotConstObject(Keeper::Type type, int id)
{
    std::vector<Keeper::Objects*> vec = ObjectsList.at(type);

    auto it = std::find_if(vec.begin(), vec.end(), [id](Keeper::Objects* o) { return o->GetID() == id; });

    return *it;
}
Keeper::Objects* Keeper::Base::GetNotConstObject(Keeper::Type type, const std::string& str)
{
    std::vector<Keeper::Objects*> vec = ObjectsList.at(type);

    auto it = std::find_if(vec.begin(), vec.end(), [str](Keeper::Objects* o) { return o->GetParsedID() == str; });

    return *it;
}

void Keeper::Base::UpdateObjectNames()
{
    if (!ObjectNames.empty()) {
        ObjectNames.clear();
    }
    ObjectNames.reserve(GetObjectsSize());
    for (auto& it : ObjectsList) {
        if (it.first == Keeper::Type::GIZMO) continue;
        for (Keeper::Objects* obj : it.second) {
            std::string objname = "";
            std::string objtype = "";

            if (obj->GetType() == Keeper::Type::CAMERA) {
                objtype = "Camera";
            }
            if (obj->GetType() == Keeper::Type::NPC) {
                objtype = "NPC";
            }
            if (obj->GetType() == Keeper::Type::SPRITE) {
                objtype = "Sprite";
            }
            if (obj->GetType() == Keeper::Type::LIGHT) {
                objtype = "Light";
            }

            std::string def = obj->GetParsedID();
            if (def.empty()) {
                def = std::to_string(obj->GetID());
            }
            objname = objtype + ": " + def;
            ObjectNames.emplace_back(objname);
        }
    }

}

bool Keeper::Base::EraseSelected()
{
    int ind = SelectedID;
    auto it = std::find_if(ObjectsList[Keeper::GIZMO].begin(),  ObjectsList[Keeper::GIZMO].end(), [ind](const Keeper::Objects* obj) { return ind == obj->GetID(); } );
    if (it != ObjectsList[Keeper::GIZMO].end()) {
        return false;
    }

    for (auto& it : ObjectsList) {
        if (it.first != Keeper::NPC && it.first != Keeper::SPRITE && it.first != Keeper::LIGHT) continue;
        std::erase_if(it.second, [ind](const Keeper::Objects* obj) { return ind == obj->GetID(); } );
    }
    for (Keeper::Objects* obj : ObjectsList[Keeper::Type::GIZMO]) {
        obj->SetHandle(nullptr);
        obj->SetVisible(false);
    }

    return true;
}

void Keeper::Base::VerifyNewObject()
{
    if (NewObjectInfo.ParsedID.empty()) {
        NewObjectInfo.ParsedID = NewObjectInfo.Type;
    }
    if (NewObjectInfo.Texture.empty()) {
        NewObjectInfo.Texture = Gfx::Renderer::GetInstance()->GetTextureMap().begin()->first;
    }
    if (NewObjectInfo.Model.empty()) {
        NewObjectInfo.Model = *Gfx::Model::GetInstance()->GetModelNames().begin();
    }
    else {
        if (!Gfx::Model::GetInstance()->GetModel(NewObjectInfo.Model)) {
            Gfx::Model::GetInstance()->LoadModel("./models/" + NewObjectInfo.Model);
        }
    }
    if (NewObjectInfo.Material.empty()) {
        NewObjectInfo.Material = "models";//*Gfx::Pipeline::GetInstance()->GetMaterialNames().begin();
    }

    if (NewObjectInfo.Type == "Light") {
        NewObjectInfo.IsLight = true;
        NewObjectInfo.Texture = "dummy.png";
    }
    else if (NewObjectInfo.Type == "Sprite") {
        NewObjectInfo.Material = "sprites";
        NewObjectInfo.Model.clear();
    }

    int count = 0;
    std::vector<std::string> objs = ObjectNames;
    std::sort(objs.begin(), objs.end());
    std::vector<std::string>::iterator it = objs.begin();
    for (int i = 0; i < ObjectTypes.size(); i++) {
        std::string name = ObjectTypes[i] + ": " + NewObjectInfo.ParsedID;
        size_t name_length = name.size();
        it = std::find_if(objs.begin(), objs.end(), [name](const auto& n) { return n == name; });
        
        //auto items = std::equal_range(std::begin(objs), std::end(objs), name);
        //size_t num = std::distance(items.first, items.second);
        if (it != objs.end()) {
            for (; it != objs.end(); ++it) {
                if (it->compare(0, name_length, name) != 0) {
                    break;
                }
                count++;
            } 
        }

        objs.erase(objs.begin(), it);
    }
    if (count != 0) {
        NewObjectInfo.ParsedID += "_" + std::to_string(count);
        printf("[%s][%d]-----\n", std::to_string(count).c_str(), count);
    }
        
    printf("inserted object:\n Name:%s\n Material: %s\n Model: %s\n Texture: %s\n, Type: %s\n", NewObjectInfo.ParsedID.c_str(),NewObjectInfo.Material.c_str(),NewObjectInfo.Model.c_str(), NewObjectInfo.Texture.c_str(), NewObjectInfo.Type.c_str() );
}

void Keeper::Base::ClearNewObjectInfo()
{
    NewObjectInfo.IsModel = false;
    NewObjectInfo.Model.clear();
    NewObjectInfo.Animations.clear();
    NewObjectInfo.Color = Vector3<float>(0,0,0);
    NewObjectInfo.Position = Vector3<float>(0,0,0);
    NewObjectInfo.LightType.clear();
    NewObjectInfo.IsLight = false;;
    NewObjectInfo.Material.clear();
    NewObjectInfo.Mesh.clear();
    NewObjectInfo.Texture.clear();
}

Keeper::Base::Base()
{
    ObjectTypes = { "NPC", "Light", "Sprite" };

    Keeper::Info info;
    info.Fragment = "gizmo.frag";
    info.Vertex = "gizmo.vert";
    info.IsModel = true;
    info.Model = "axisy.obj";
    info.Material = "gizmo";
    info.Position = Vector3<float>(0.0f);
    info.Texture = "dummy.png";

    GizmoInfo[Keeper::Gizmo::Type::Y] = info;

    info.Model = "axisx.obj";
    GizmoInfo[Keeper::Gizmo::Type::X] = info;
    info.Model = "axisz.obj";
    GizmoInfo[Keeper::Gizmo::Type::Z] = info;

    Keeper::Info modules;
    modules.IsModel = false;
    modules.Position = Vector3<float>(0.0f, 0.0f, 0.0f);
    modules.Material = "intro";
    modules.Mesh = "dummy";
    DefaultObjects[Infos::INFO_INTRO] = modules;
    modules.Material = "particles-draw";
    DefaultObjects[Infos::INFO_PARTICLES] = modules;
    modules.Material = "compute_mtx";
    DefaultObjects[Infos::INFO_COMPUTE_MTX] = modules;
    modules.Material = "visibility_compute";
    DefaultObjects[Infos::INFO_VISIBILITY_COMPUTE] = modules;
    modules.Material = "compute_skinning";
    DefaultObjects[Infos::INFO_COMPUTE_SKINNING] = modules;

    modules.Material = "grid";
    modules.Texture = "dummy.png";
    modules.VertexBase = true;
    modules.Mesh = "";
    DefaultObjects[Infos::INFO_GRID] = modules;

    modules.Material = "outline";
    modules.Texture = "mask";
    modules.Mesh = "dummy.png";
    DefaultObjects[Infos::INFO_OUTLINE] = modules;
    DefaultObjects[Infos::INFO_MASK] = modules;
    modules.VertexBase = false;
    modules.Material = "gbuffer";
    modules.Mesh = "dummy.png";
    modules.Texture = "albedo";
    DefaultObjects[Infos::INFO_GBUFFER] = modules;
    modules.Texture = "";
    modules.Material = "shadows";
    DefaultObjects[Infos::INFO_SHADOWS] = modules;
    modules.Texture = "cubemaps/skybox";
    modules.Material = "skybox";
    modules.Model = "cube.obj";
    DefaultObjects[Infos::INFO_SKYBOX] = modules;

    modules.Model = "";
    modules.Material = "lighting";
    modules.Texture = "cubemaps/skybox";
    modules.Textures.reserve(4);
    modules.Textures.push_back("normal");
    modules.Textures.push_back("position");
    modules.Textures.push_back("albedo");
    modules.Textures.push_back("shadows");
    modules.Mesh = "dummy";
    DefaultObjects[Infos::INFO_LIGHTING] = modules;

}

void Keeper::Base::Update()
{
#ifdef TRACY
    ZoneScopedN("Keeper::Base::Update");
#endif
    std::vector<Objects*>::iterator camera_it = std::find_if(ObjectsList[Keeper::Type::CAMERA].begin(), ObjectsList[Keeper::Type::CAMERA].end(), [](const Keeper::Objects* obj) { return obj->CameraType() == static_cast<uint32_t>(Keeper::Camera::Type::EDITOR); });
    Keeper::Camera* camera = reinterpret_cast<Keeper::Camera*>(*camera_it);

    glm::mat4 view = glm::lookAt(camera->GetPos(), camera->GetPos() + camera->GetFront(), camera->GetUp());
	glm::mat4 projection = glm::perspective(glm::radians(45.f), float(Gfx::Device::GetInstance()->GetSwapchainSize().x) / float(Gfx::Device::GetInstance()->GetSwapchainSize().y), 0.01f, 100.0f);

    // for (auto& it : ObjectsList) {
    //     if (it.first == Keeper::Type::GIZMO || it.first == Keeper::Type::CAMERA || it.first == Keeper::Type::SPRITE) continue;
    //
    //     for (Keeper::Objects* obj : it.second) {
    //         if (obj->GetModelName().empty()) continue;
    //         Thread::Pool::GetInstance()->Push({
    //         Thread::Task::Name::UPDATE, Thread::Task::Type::EXECUTE, [this, projection, view](int i, Keeper::Objects* obj) {
    //
    //         bool visible = Keeper::Collision::Cull(projection * view, obj);
    //     //printf("----%s|%d\n", obj->GetModelName().c_str(), visible);
    //         obj->SetVisible(visible);}, {}, 0, obj, {} });
    //     }
    // }

    int id = SelectedID;
    std::vector<Objects*>::iterator light_it = ObjectsList[Keeper::Type::LIGHT].end();
    std::vector<Objects*>::iterator selected_it = std::find_if(ObjectsList[Keeper::Type::NPC].begin(), ObjectsList[Keeper::Type::NPC].end(), [id](const Keeper::Objects* obj) { return obj->IsVisible() && (obj->GetID() == id); });
    if (selected_it == ObjectsList[Keeper::Type::NPC].end()) {
        light_it = std::find_if(ObjectsList[Keeper::Type::LIGHT].begin(), ObjectsList[Keeper::Type::LIGHT].end(), [id](const Keeper::Objects* obj) { return obj->IsVisible() && (obj->GetID() == id); });
    }
    
    // if (selected_it != ObjectsList[Keeper::Type::NPC].end()) {
    //
    //     printf("id %d| selected ID %d!!!\n", (*selected_it)->GetID(), SelectedID);
    // }
    //
    // if (light_it != ObjectsList[Keeper::Type::LIGHT].end())
    // {
    //     printf("Light: id %d| selected ID %d!!!\n", (*light_it)->GetID(), SelectedID);
    // }
    std::vector<Objects*>::iterator gizmo_it = std::find_if(ObjectsList[Keeper::Type::GIZMO].begin(), ObjectsList[Keeper::Type::GIZMO].end(), [id](const Keeper::Objects* obj) { return obj->GetID() == id;});

    for (Keeper::Objects* obj : ObjectsList[Keeper::Type::GIZMO]) {
        if (selected_it != ObjectsList[Keeper::Type::NPC].end()) {
            obj->SetVisible(true);
            obj->SetHandle(*selected_it);
            obj->SetPosition((*selected_it)->GetPosition());
        }
        if ( light_it != ObjectsList[Keeper::Type::LIGHT].end()) {
            obj->SetVisible(true);
            obj->SetHandle(*light_it);
            obj->SetPosition((*light_it)->GetPosition());
        }
        if ((selected_it == ObjectsList[Keeper::Type::NPC].end() && light_it == ObjectsList[Keeper::Type::LIGHT].end()) && gizmo_it == ObjectsList[Keeper::Type::GIZMO].end()) {
            obj->SetVisible(false);
            obj->SetHandle(0);
        }
        obj->Update();
    }

    for (auto& it : ObjectsList) {
        if (it.first == Keeper::Type::GIZMO) continue;
        if ((gizmo_it != ObjectsList[Keeper::Type::GIZMO].end())&& it.first == Keeper::Type::CAMERA) continue;
        for (Keeper::Objects* obj : it.second) {
            if (!obj->IsVisible()) continue;
            Keeper::Objects* gizmo_handle = nullptr;

            obj->SetSelected(obj->GetID() == SelectedID);
            if (gizmo_it != ObjectsList[Keeper::Type::GIZMO].end() && (*gizmo_it)->GetHandle() && (*gizmo_it)->GetHandle()->GetID() == obj->GetID()) {
                    gizmo_handle = *gizmo_it;
                    obj->SetSelected(true);
            }
            else {
                    obj->SetSelected(false);
                }
            obj->SetGizmo(gizmo_handle);
            obj->Update();
        }
    }
    
 }

void Keeper::Base::SpawnObjects(const Keeper::Info& info)
{
    Vector3<float> offsets = info.Offset;
    ASSERT(offsets.x == 0 && offsets.y == 0 && offsets.z == 0, "Keeper::Base::SpawnObjects(): offsets can't be 0");

    Keeper::Info spawn = info;
    int i = 0;
    float off = 0.1;
    for (float x = info.Position.x; x < info.Position.x + offsets.x; x += 1.0f ) {
        for (float y = info.Position.y; y < info.Position.y + offsets.y; y += 1.0f ) {
            for (float z = info.Position.z; z < info.Position.z + offsets.z; z += 1.0f ) {
                spawn.Position = Vector3<float>( x, y, z );
                if (!spawn.ParsedID.empty()) {
                    spawn.ParsedID = info.ParsedID + "_" + std::to_string(i);
                    i++;
                }
                spawn.Spawn = true;
                // spawn.AnimOffset = off;
                off += 0.0001;
                Create<Keeper::Npc>(new Keeper::Npc(spawn));
            }
        }
    }
}

void Keeper::Base::Create(const std::vector<Keeper::Info>& info)
{
    int lighti = 0;
    for (const Keeper::Info& obj : info) {
        if (obj.IsLight) {
            std::string tag = obj.ParsedID;
            if (tag.empty()) {
                tag = "Light_" + std::to_string(lighti);
                lighti++;
            }
            Create<Keeper::Light>(new Keeper::Light(obj, tag));
        }
        else if (obj.IsModel) {
            if (obj.Spawn) {
                SpawnObjects(obj);
            }
            else {
                Create<Keeper::Npc>(new Keeper::Npc(obj));
            }
        }
        else {
            Create<Keeper::Sprite>(new Keeper::Sprite(obj));
        }
    }
}

bool Keeper::Base::Find(Keeper::Type type) const
{
    return ObjectsList.find(type) != ObjectsList.end();
}

size_t Keeper::Base::GetObjectsSize() const
{
    size_t size = 0;
    for (auto& it : ObjectsList) {
        size += it.second.size();
    }
    return size;
}
