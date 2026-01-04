#include "anthraxAI/gamemodules/modules.h"
#include "anthraxAI/core/scene.h"
#include "anthraxAI/gameobjects/gameobjects.h"
#include "anthraxAI/gfx/model.h"
#include "anthraxAI/gfx/renderhelpers.h"
#include "anthraxAI/gfx/vkdefines.h"
#include "anthraxAI/gfx/vkdescriptors.h"
#include "anthraxAI/gfx/vkrendertarget.h"
#include "anthraxAI/utils/defines.h"

#include "anthraxAI/gfx/vkpipeline.h"
#include "anthraxAI/gfx/vkrenderer.h"
#include <cstdint>
#include <cstdio>
#include <functional>
#include <vector>

void Modules::Base::Clear()
{
    was_vertecies_count = false;
    SceneModules.clear();
    vertex_count = 0;
    mapped_spawn_vertecies.clear();
}

void Modules::Base::Populate(const std::string& key, Modules::Info scene, std::function<bool(Keeper::Type)> skip_type)
{
    ASSERT(!GameObjects, "GameObjects is nullptr!");

    Module module(scene);
    module.SetTag(key);
    for (auto& it : GameObjects->GetObjects()) {
        for (Keeper::Objects* info : it.second) {
            if (skip_type(info->GetType())) continue;
            if (info->GetType() == Keeper::NPC) {
                module.SetGizmo(true);
            }
            QueueType type = RQ_GENERAL;
            if (info->GetType() == Keeper::LIGHT) {
                type = RQ_LIGHT;
            }
            module.AddRQ(type, LoadResources(info));
        }
    }
    SceneModules[key] = module;
}

void Modules::Base::Populate(const std::string& key, Info scene, std::vector<Keeper::Objects*> vec)
{
    ASSERT(!GameObjects, "GameObjects is nullptr!");

    Module module(scene);
    module.SetTag(key);
    for (auto& info : vec) {
            if (info->GetType() == Keeper::NPC) {
                module.SetGizmo(true);
            }
            QueueType type = RQ_GENERAL;
            if (info->GetType() == Keeper::LIGHT) {
                type = RQ_LIGHT;
            }
            module.AddRQ(type, LoadResources(info));
    }
    SceneModules[key] = module;
 
}

void Modules::Base::Populate(const std::string& key, Modules::Info scene, Keeper::Info info)
{
    Module module(scene);

    module.SetTag(key);

    Gfx::RenderObject rqobj;
    rqobj.Position = Vector3<float>(0.0f);
    rqobj.MaterialName = info.Material;
    rqobj.Material = Gfx::Pipeline::GetInstance()->GetMaterial(info.Material);
    if (key == "particles" || key == "compute_mtx"|| key == "compute_skinning") {
        rqobj.IsCompute = true;
        module.AddRQ(RQ_GENERAL, rqobj);
        SceneModules[key] = module;
        return;
    }
    if (info.Texture == "mask") {
        rqobj.output_vertex = true; 
        rqobj.Texture = Gfx::Renderer::GetInstance()->GetRT(Gfx::GetKey(info.Texture));
    }
    if (info.Texture == "albedo") {
        rqobj.Texture = Gfx::Renderer::GetInstance()->GetRT(Gfx::GetKey(info.Texture));
    }
    if (key == "skybox") {
        rqobj.input_vertex = true; 
        rqobj.ModelName = info.Model; 
        rqobj.TextureName =  info.Texture;
        rqobj.Texture = Gfx::Renderer::GetInstance()->GetCubemap(info.Texture);
        rqobj.Model[0] = Gfx::Model::GetInstance()->GetModel(info.Model);
        rqobj.Model[1] = Gfx::Model::GetInstance()->GetModel(info.Model);
        rqobj.Model[2] = Gfx::Model::GetInstance()->GetModel(info.Model);
    }
    if (!info.Textures.empty()) {
        if (!CubemapTexture.empty()) {
            rqobj.TextureName =  CubemapTexture;
            rqobj.Texture = Gfx::Renderer::GetInstance()->GetCubemap(CubemapTexture);
        }
        else if (!info.Texture.empty()) {
            rqobj.TextureName =  info.Texture;
            CubemapTexture = info.Texture;
            rqobj.Texture = Gfx::Renderer::GetInstance()->GetCubemap(info.Texture);
        }
        rqobj.Textures.resize(info.Textures.size());
        int i = 0;
        for (Gfx::RenderTarget*& rt : rqobj.Textures) {
            rt = Gfx::Renderer::GetInstance()->GetRT(Gfx::GetKey(info.Textures[i]));
            i++;
        }
    }
    /*else if (!info.Texture.empty()) {    */
    /*    rqobj.Texture = Gfx::Renderer::GetInstance()->GetTexture(info.Texture);*/
    /*}*/
    rqobj.Mesh = Gfx::Mesh::GetInstance()->GetMesh(info.Mesh);
    rqobj.VertexBase = info.VertexBase;
    rqobj.IsVisible = true;

    module.AddRQ(RQ_GENERAL, rqobj);

    SceneModules[key] = module;
    float offsets = 0;
    std::string tmp_spawn;
    if (key == "mask" || key == "gbuffer" || key == "shadows") {
        Modules::RenderQueueVec rq = SceneModules[CurrentScene].GetRenderQueue(RQ_GENERAL);
        Modules::RenderQueueVec rq_skinned;
        for (Gfx::RenderObject& obj : rq) {
            obj.MaterialName = key;
            obj.Material =  Gfx::Pipeline::GetInstance()->GetMaterial(obj.MaterialName);
#ifdef COMPUTE_SKINNING
            if (key == "mask") {
                obj.output_vertex = true; 
            }

            if (key == "shadows" || key == "gbuffer") {
                obj.skinned_vertex = true; 
            }
            if (!was_vertecies_count && key == "gbuffer") {
                if (tmp_spawn != obj.SpawnName) {
                    for (auto& info : obj.Model[0]->Meshes) {
                        vertex_count += info->Vertices.size(); 
                    }
                    mapped_spawn_vertecies[obj.SpawnName] = vertex_count;
                    tmp_spawn = obj.SpawnName;
                    rq_skinned.push_back(obj);
                }
            }
#endif
        }
        SetRenderQueue(RQ_GENERAL, key, rq);
        rq = SceneModules[CurrentScene].GetRenderQueue(RQ_LIGHT);
        if (rq.empty() || key == "shadows") {
            if (key == "gbuffer" && !was_vertecies_count) was_vertecies_count = true; 
            SetSkinnginRenderQueue(key, rq_skinned);
            return;
        }
        
        for (Gfx::RenderObject& obj : rq) {
            obj.MaterialName = key;
            obj.Material =  Gfx::Pipeline::GetInstance()->GetMaterial(obj.MaterialName);
#ifdef COMPUTE_SKINNING
            if (key == "mask") {
                obj.output_vertex = true; 
            }

            if (key == "shadows" || key == "gbuffer") {
            obj.skinned_vertex = true; 
            }
            if (!was_vertecies_count && key == "gbuffer") {
                if (tmp_spawn != obj.SpawnName) {
                    for (auto& info : obj.Model[0]->Meshes) {
                        vertex_count += info->Vertices.size(); 
                    }
                    mapped_spawn_vertecies[obj.SpawnName] = vertex_count;
                    tmp_spawn = obj.SpawnName;
                    rq_skinned.push_back(obj);
                }
            }

            // if (key == "compute_skinning") {
            // for (auto& info : obj.Model[0]->Meshes) {
            //     vertex_count += info->Vertices.size(); 
            // }
            // }
#endif
        }
        SetRenderQueue(RQ_LIGHT, key, rq);
        SetSkinnginRenderQueue(key, rq_skinned);
    }
    was_vertecies_count = true;
}

void Modules::Base::EraseSelected()
{
    Gfx::RenderObject rem;
    QueueType type = RQ_GENERAL;
    int ind = GameObjects->GetSelectedID();
    for (auto& it : SceneModules) {
        if (it.first != "shadows" && it.first != "gbuffer" && it.first != "mask" && it.first != CurrentScene) continue;

         int num = std::erase_if(it.second.GetRenderQueue(RQ_GENERAL), [ind](const Gfx::RenderObject& obj) { return ind == obj.ID; } );
        if (num != 0) {
            printf("info about eraser obj in module: type[%s] tag[%s]\n", it.first.c_str(), it.second.GetTag().c_str());
                 }
        else {
num = std::erase_if(it.second.GetRenderQueue(RQ_LIGHT), [ind](const Gfx::RenderObject& obj) { return ind == obj.ID; } );
        if (num != 0) {
            printf("info about eraser obj in module: type[%s] tag[%s]\n", it.first.c_str(), it.second.GetTag().c_str());
        }

        }
    }
//        int num = std::erase_if(SceneModules["gbuffer"].GetRenderQueue(RQ_GENERAL), [ind](const Gfx::RenderObject& obj) { return ind == obj.ID; } );
//        num = std::erase_if(SceneModules[CurrentScene].GetRenderQueue(RQ_GENERAL), [ind](const Gfx::RenderObject& obj) { return ind == obj.ID; } );
//        num = std::erase_if(SceneModules["mask"].GetRenderQueue(RQ_GENERAL), [ind](const Gfx::RenderObject& obj) { return ind == obj.ID; } );
        //if (num == 0) {
        //    num = std::erase_if(it.second.GetRenderQueue(RQ_LIGHT), [ind](const Gfx::RenderObject& obj) { return ind == obj.ID; } );
        //}
    //}

}

void Modules::Base::Insert(const Keeper::Objects* obj)
{
    QueueType type = RQ_GENERAL;
    if (obj->GetType() == Keeper::LIGHT) {
        type = RQ_LIGHT;
    }

    if (obj->GetType() != Keeper::SPRITE) {
        SceneModules[CurrentScene].AddRQ(type, LoadResources(obj));
        UpdateResource(SceneModules[CurrentScene], SceneModules[CurrentScene].GetRenderQueue(type).at(SceneModules[CurrentScene].GetRenderQueue(type).size() - 1));
        Gfx::RenderObject robj = SceneModules[CurrentScene].GetRenderQueue(type).at(SceneModules[CurrentScene].GetRenderQueue(type).size() - 1);
        robj.MaterialName = "gbuffer";
        robj.Material = Gfx::Pipeline::GetInstance()->GetMaterial(robj.MaterialName);
        SceneModules["gbuffer"].AddRQ(type, robj);

        robj.MaterialName = "mask";
        robj.Material = Gfx::Pipeline::GetInstance()->GetMaterial(robj.MaterialName);
        SceneModules["mask"].AddRQ(type, robj);

        robj.MaterialName = "shadows";
        robj.Material = Gfx::Pipeline::GetInstance()->GetMaterial(robj.MaterialName);
        SceneModules["shadows"].AddRQ(type, robj);
    }
    else {
        std::string key = "sprite";
        if (!Find("sprite")) {
            Modules::Info info;
            info.BindlessType = Gfx::BINDLESS_DATA_CAM_STORAGE_SAMPLER ;
            info.IAttachments.Add(Gfx::RT_MAIN_COLOR);
            SceneModules[key] = info;
        }
        SceneModules[key].AddRQ(type, LoadResources(obj));
        UpdateResource(SceneModules[key], SceneModules[key].GetRenderQueue(type).at(SceneModules[key].GetRenderQueue(type).size() - 1));
    }
}

void Modules::Base::RestartAnimator()
{
    if (Animator) {
        delete Animator;
    }
    Animator = new Core::AnimatorBase();

    Utils::Debug::GetInstance()->AnimStartMs = Engine::GetInstance()->GetTime();
    Animator->Init();
}

void Modules::Base::UpdateResource(Modules::Module& module, Gfx::RenderObject& obj)
{
    switch (module.GetBindlessType()) {
        case Gfx::BINDLESS_DATA_CAM_STORAGE_SAMPLER: {
            for (int i = 0; i < MAX_FRAMES; i++) {
            if (!obj.Textures.empty()) {
                std::vector<Gfx::RenderTarget*>::iterator it = obj.Textures.begin();
                obj.TextureBind[i] = Gfx::DescriptorsBase::GetInstance()->UpdateTexture((*it)->GetImageView(), *((*it)->GetSampler()), (*it)->GetName(), i);
                it++;
                for (; it != obj.Textures.end(); ++it) {
                    ASSERT(!(*it), "Modules::Base::UpdateResource() invalid render target pointer!");
                    Gfx::DescriptorsBase::GetInstance()->UpdateTexture((*it)->GetImageView(), *((*it)->GetSampler()), (*it)->GetName(), i);
                }
                if (obj.Texture) {
                    CubemapBind[i] = Gfx::DescriptorsBase::GetInstance()->UpdateTexture(obj.Texture->GetImageView(), *(obj.Texture->GetSampler()), obj.Texture->GetName(), i);
                }
            }
            else {
                ASSERT(!obj.Texture, "Modules::Base::UpdateResource() invalid render target pointer!");
                obj.TextureBind[i] = Gfx::DescriptorsBase::GetInstance()->UpdateTexture(obj.Texture->GetImageView(), *(obj.Texture->GetSampler()), obj.Texture->GetName(), i);
            }
    	    obj.BufferBind[i] = Gfx::DescriptorsBase::GetInstance()->UpdateBuffer(Gfx::DescriptorsBase::GetInstance()->GetCameraBuffer(i), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Gfx::DescriptorsBase::GetInstance()->GetCameraUBO(i).tag, i);
            obj.StorageBind[i] = Gfx::DescriptorsBase::GetInstance()->UpdateBuffer(Gfx::DescriptorsBase::GetInstance()->GetStorageBuffer(i), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, Gfx::DescriptorsBase::GetInstance()->GetStorageUBO(i).tag, i);
            obj.InstanceBind[i] = Gfx::DescriptorsBase::GetInstance()->UpdateCompute(Gfx::DescriptorsBase::GetInstance()->GetInstanceBuffer(i), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, Gfx::DescriptorsBase::GetInstance()->GetInstanceUBO(i).tag, i);
            }
            obj.HasStorage = obj.Model[0] ? true : false;
            module.SetCameraBuffer(true);
            module.SetStorageBuffer(true);
            module.SetTexture(true);

            break;
        }
        case Gfx::BINDLESS_DATA_PARTICLES: {
            for (int i = 0; i < MAX_FRAMES; i++) {
    	        obj.StorageBind[i] = Gfx::DescriptorsBase::GetInstance()->UpdateCompute(Gfx::DescriptorsBase::GetInstance()->GetComputeBuffer(i), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, Gfx::DescriptorsBase::GetInstance()->GetComputeUBO(i).tag,i);
            }
            break;
        }
#ifdef COMPUTE_SKINNING
        case Gfx::BINDLESS_DATA_SKINNING: {
            for (int i = 0; i < MAX_FRAMES; i++) {
                obj.SkinningHelperBind[i] = Gfx::DescriptorsBase::GetInstance()->UpdateCompute(Gfx::DescriptorsBase::GetInstance()->GetBuffer(0, Gfx::SKINNING_HELPER), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, Gfx::DescriptorsBase::GetInstance()->GetUBO(0, Gfx::SKINNING_HELPER).tag, i);

    	        obj.BufferBind[i] = Gfx::DescriptorsBase::GetInstance()->UpdateBuffer(Gfx::DescriptorsBase::GetInstance()->GetCameraBuffer(i), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Gfx::DescriptorsBase::GetInstance()->GetCameraUBO(i).tag, i);
    	        obj.StorageBind[i] = Gfx::DescriptorsBase::GetInstance()->UpdateCompute(Gfx::DescriptorsBase::GetInstance()->GetBuffer(0, Gfx::SKINNING_IN), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, Gfx::DescriptorsBase::GetInstance()->GetUBO(0, Gfx::SKINNING_IN).tag, i);
                obj.InstanceBind[i] = Gfx::DescriptorsBase::GetInstance()->UpdateCompute(Gfx::DescriptorsBase::GetInstance()->GetBuffer(0, Gfx::SKINNING_OUT), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, Gfx::DescriptorsBase::GetInstance()->GetUBO(0, Gfx::SKINNING_OUT).tag, i);
    	        obj.TextureBind[i] = Gfx::DescriptorsBase::GetInstance()->UpdateCompute(Gfx::DescriptorsBase::GetInstance()->GetInstanceBuffer(i), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, Gfx::DescriptorsBase::GetInstance()->GetInstanceUBO(i).tag, i);
            }
            break;
        }

#endif    
#ifdef COMPUTE_MTX
        case Gfx::BINDLESS_DATA_STORAGE: {
            for (int i = 0; i < MAX_FRAMES; i++) {
    	        obj.BufferBind[i] = Gfx::DescriptorsBase::GetInstance()->UpdateBuffer(Gfx::DescriptorsBase::GetInstance()->GetCameraBuffer(i), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Gfx::DescriptorsBase::GetInstance()->GetCameraUBO(i).tag, i);
    	        obj.StorageBind[i] = Gfx::DescriptorsBase::GetInstance()->UpdateCompute(Gfx::DescriptorsBase::GetInstance()->GetInstanceBuffer(i), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, Gfx::DescriptorsBase::GetInstance()->GetInstanceUBO(i).tag, i);
                obj.InstanceBind[i] = Gfx::DescriptorsBase::GetInstance()->UpdateCompute(Gfx::DescriptorsBase::GetInstance()->GetAnimationBuffer(i), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, Gfx::DescriptorsBase::GetInstance()->GetAnimationUBO(i).tag, i);
            Gfx::Renderer::GetInstance()->set_global_animation_buffer(obj.InstanceBind[i], i);
            }
            break;
        }
#endif // COMPUTE_MTX
        case Gfx::BINDLESS_DATA_CAM_BUFFER: {
            for (int i = 0; i < MAX_FRAMES; i++) {
    	    obj.BufferBind[i] = Gfx::DescriptorsBase::GetInstance()->UpdateBuffer(Gfx::DescriptorsBase::GetInstance()->GetCameraBuffer(i), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Gfx::DescriptorsBase::GetInstance()->GetCameraUBO(i).tag,i);
            }
            module.SetCameraBuffer(true);
            break;
        }
        default:
            break;
    }

}

void Modules::Base::UpdateResources()
{
    for (auto& it : SceneModules) {
        for (auto& it_v : it.second.GetRenderQueueMap()) {
        for (Gfx::RenderObject& obj : it_v.second) {
            UpdateResource(it.second, obj);
        }
        }
    }
    for (auto& it : SceneModules) {
        // for (auto& it_v : it.second.GetRenderQueueMap()) {
        for (Gfx::RenderObject& obj : it.second.GetSkinningRQ()) {
            UpdateResource(it.second, obj);
        }
        // }
    }
}

void Modules::Base::UpdateMaterials()
{
    for (auto& it : SceneModules) {
        for (auto& it_v : it.second.GetRenderQueueMap()) {
        
        for (Gfx::RenderObject& obj : it_v.second) {

            obj.Material = Gfx::Pipeline::GetInstance()->GetMaterial(obj.MaterialName);
        }
        }
    }
}

void Modules::Base::ThreadedRQ(int i, Keeper::Objects* info)
{ 
#ifdef TRACY
    ZoneScopedN("Modules::Base::ThreadedRQ");
#endif
    QueueType type = RQ_GENERAL;
    if (info->GetType() == Keeper::LIGHT) {
        type = RQ_LIGHT;
    }

    SceneModules[CurrentScene].GetRenderQueue(type)[i].IsSelected = info->IsVisible() && (info->GetGizmo() || SceneModules[CurrentScene].GetRenderQueue(type)[i].ID == GameObjects->GetSelectedID() ? 1 : 0);
    SceneModules["mask"].GetRenderQueue(type)[i].IsSelected = info->IsVisible() && SceneModules[CurrentScene].GetRenderQueue(type)[i].IsSelected;//info->GetGizmo() || SceneModules[CurrentScene].GetRenderQueue()[i].ID == GameObjects->GetSelectedID() ? 1 : 0;
    if (SceneModules["mask"].GetRenderQueue(type)[i].IsSelected) {
        HasOutline = true;
    }
    SceneModules[CurrentScene].GetRenderQueue(type)[i].IsVisible = info->IsVisible();
    SceneModules[CurrentScene].GetRenderQueue(type)[i].Position = info->GetPosition();
    SceneModules[CurrentScene].GetRenderQueue(type)[i].rotation = info->GetRotation();
    SceneModules["gbuffer"].GetRenderQueue(type)[i].IsSelected =SceneModules[CurrentScene].GetRenderQueue(type)[i].IsSelected ; 
    SceneModules["gbuffer"].GetRenderQueue(type)[i].IsVisible = info->IsVisible();
    SceneModules["gbuffer"].GetRenderQueue(type)[i].Position = info->GetPosition();
    SceneModules["gbuffer"].GetRenderQueue(type)[i].rotation = info->GetRotation();
#ifndef COMPUTE_MTX
    if (SceneModules[CurrentScene].GetRenderQueue(type)[i].IsVisible && HasAnimation(SceneModules[CurrentScene].GetRenderQueue(type)[i].ID)) {
        Animator->Update(SceneModules[CurrentScene].GetRenderQueue(type)[i]);
    }
#endif
}

void Modules::Base::UpdateRQ()
{
#ifdef TRACY
    ZoneScopedN("Modules::Base::UpdateRQ");
#endif
    if (GameObjects->IsValid(Keeper::Type::NPC)) {
        int i = 0;

        const std::vector<Keeper::Objects*>& npc = GameObjects->Get(Keeper::Type::NPC);
        for (Keeper::Objects* info : npc) {
#ifndef COMPUTE_MTX
            if (Thread::Pool::GetInstance()->IsInit()) {
            Thread::Pool::GetInstance()->Push({
            Thread::Task::Name::UPDATE, Thread::Task::Type::EXECUTE, [this](int i, Keeper::Objects* info) {
                ThreadedRQ(i, info); }, {},{}, {}, i, info, {} });
            }
            else {
                ThreadedRQ(i, info);
            }
#else 
                ThreadedRQ(i, info);
#endif
            i++;
        }
#ifndef COMPUTE_MTX
        if (Thread::Pool::GetInstance()->IsInit()) {
            Thread::Pool::GetInstance()->WaitWork();
        }
#endif
        i = 0;
        const std::vector<Keeper::Objects*> light = GameObjects->Get(Keeper::Type::LIGHT);
        for (Keeper::Objects* info : light) {
            ThreadedRQ(i, info);
            i++;
        }
        i = 0;
        const std::vector<Keeper::Objects*> gizmo = GameObjects->Get(Keeper::Type::GIZMO);
        for (Keeper::Objects* info : gizmo) {
            SceneModules["gizmo"].GetRenderQueue(RQ_GENERAL)[i].IsVisible = info->IsVisible();
            SceneModules["gizmo"].GetRenderQueue(RQ_GENERAL)[i].Position = info->GetPosition();
           // SceneModules["gizmo"].GetRenderQueue(RQ_GENERAL)[i].IsSelected = info->IsSelected();
            i++;
        }
    }
}

bool Modules::Base::UpdateTexture(const std::string& str, Core::ImGuiHelper::TextureForUpdate upd)
{
    int id = upd.ID;
    auto it = std::find_if(SceneModules[str].GetRenderQueue(RQ_GENERAL).begin(), SceneModules[str].GetRenderQueue(RQ_GENERAL).end(), [id](Gfx::RenderObject& obj) { return obj.ID == id; });
    if (upd.Cubemap) {
        it = SceneModules[str].GetRenderQueue(RQ_GENERAL).begin();
    }
    if (it != SceneModules[str].GetRenderQueue(RQ_GENERAL).end()) {
        for (int i = 0; i < MAX_FRAMES; i++) {
            if (!it->Textures.empty()) {
                std::vector<Gfx::RenderTarget*>::iterator texture_it = it->Textures.begin();
                it->TextureBind[i] = Gfx::DescriptorsBase::GetInstance()->UpdateTexture((*texture_it)->GetImageView(), *((*texture_it)->GetSampler()), (*texture_it)->GetName(), i);
                texture_it++;
                for (; texture_it != it->Textures.end(); ++texture_it) {
                    ASSERT(!(*texture_it), "Modules::Base::UpdateResource() invalid render target pointer!");
                    Gfx::DescriptorsBase::GetInstance()->UpdateTexture((*texture_it)->GetImageView(), *((*texture_it)->GetSampler()), (*texture_it)->GetName(), i);
                }
                if (it->Texture) {
                    it->Texture = Gfx::Renderer::GetInstance()->GetCubemap(upd.NewTextureName);
                    CubemapTexture = upd.NewTextureName;
                    it->TextureName = upd.NewTextureName;
                    CubemapBind[i] = Gfx::DescriptorsBase::GetInstance()->UpdateTexture(it->Texture->GetImageView(), *(it->Texture->GetSampler()), it->Texture->GetName(), i);
                }

            }
            else {
                it->Texture = Gfx::Renderer::GetInstance()->GetTexture(upd.NewTextureName);
                it->TextureName = upd.NewTextureName;
                ASSERT(!it->Texture, "Modules::Base::UpdateResource() invalid render target pointer!");
                it->TextureBind[i] = Gfx::DescriptorsBase::GetInstance()->UpdateTexture(it->Texture->GetImageView(), *(it->Texture->GetSampler()), it->Texture->GetName(), i);
            }
        }
        return true;
    }
    return false;
}

void Modules::Base::UpdateTextureUIManager()
{
    if (Core::ImGuiHelper::GetInstance()->TextureNeedsUpdate()) {
        Core::ImGuiHelper::TextureForUpdate upd = Core::ImGuiHelper::GetInstance()->GetTextureForUpdate();
        if (upd.Cubemap) {
            if (SceneModules.find("lighting") != SceneModules.end()) {
                UpdateTexture("lighting", upd);
            }
        }
        else {
            if (UpdateTexture(CurrentScene, upd)) {
                if (SceneModules.find("gbuffer") != SceneModules.end()) {
                    UpdateTexture("gbuffer", upd);
                }
            }
            else {
                UpdateTexture("sprite", upd);
            }
        }

        Core::ImGuiHelper::GetInstance()->ResetTextureUpdate();
    }
}

void Modules::Base::UpdateSamplers()
{
    if (Gfx::Renderer::GetInstance()->GetUpdateSamplers()) {
        if (SceneModules.find("outline") != SceneModules.end())
        {
            Modules::Info info;
            info.BindlessType = Gfx::BINDLESS_DATA_CAM_STORAGE_SAMPLER ;
            info.IAttachments.Add(Gfx::RT_MAIN_COLOR);
            Populate("outline", info,
                GameObjects->GetInfo(Keeper::Infos::INFO_OUTLINE)
            );

            for (Gfx::RenderObject& obj : SceneModules["outline"].GetRenderQueue(RQ_GENERAL)) {
                UpdateResource(SceneModules["outline"], obj);
            }
            for (Gfx::RenderObject& obj : SceneModules["outline"].GetRenderQueue(RQ_LIGHT)) {
                UpdateResource(SceneModules["outline"], obj);
            }
        }
        if (SceneModules.find("lighting") != SceneModules.end())
        {
            Modules::Info info;
            info.BindlessType = Gfx::BINDLESS_DATA_CAM_STORAGE_SAMPLER ;
            info.IAttachments.Add(Gfx::RT_MAIN_COLOR);
            Populate("lighting", info,
                GameObjects->GetInfo(Keeper::Infos::INFO_LIGHTING)
            );

            for (Gfx::RenderObject& obj : SceneModules["lighting"].GetRenderQueue(RQ_GENERAL)) {
                UpdateResource(SceneModules["lighting"], obj);
            }
            for (Gfx::RenderObject& obj : SceneModules["lighting"].GetRenderQueue(RQ_LIGHT)) {
                UpdateResource(SceneModules["lighting"], obj);
            }

        }
        if (SceneModules.find("gbuffer") != SceneModules.end()) {
            {
            Modules::RenderQueueVec rq = SceneModules[CurrentScene].GetRenderQueue(RQ_GENERAL);
            std::string material= "gbuffer";
            Gfx::Material* mat = Gfx::Pipeline::GetInstance()->GetMaterial(material);
            for (Gfx::RenderObject& obj : rq) {
                obj.Material = mat;        
            }
            SetRenderQueue(RQ_GENERAL, "gbuffer", rq);
            }
            {
            Modules::RenderQueueVec rq = SceneModules[CurrentScene].GetRenderQueue(RQ_LIGHT);
            std::string material= "gbuffer";
            Gfx::Material* mat = Gfx::Pipeline::GetInstance()->GetMaterial(material);
            for (Gfx::RenderObject& obj : rq) {
                obj.Material = mat;        
            }
            SetRenderQueue(RQ_LIGHT, "gbuffer", rq);
            }
        }
        Gfx::Renderer::GetInstance()->SetUpdateSamplers(false);
    }

}

void Modules::Base::Update(uint32_t update_type, bool force_update)
{
    switch (update_type)
    {
        case Modules::Update::RESOURCES:
            UpdateResources();
            break;
        case Modules::Update::MATERIALS:
            UpdateMaterials();
            break;
        case Modules::Update::RQ:
            UpdateRQ();
            break;
        case Modules::Update::TEXTURE_UI_MANAGER:
            UpdateTextureUIManager();
            break;
        case Modules::Update::SAMPLERS:
            UpdateSamplers();
            break;
        default:
            break;
    }
}

Gfx::RenderObject Modules::Base::LoadResources(const Keeper::Objects* info)
{
    Gfx::RenderObject rqobj;
    if (info->GetAxis() != -1) {
       rqobj.GizmoType = info->GetAxis();
        rqobj.input_vertex = true;
    }
    
    rqobj.Spawn = info->GetSpawn();
    rqobj.instance_size = info->GetInstanceCount();
    rqobj.SpawnName = info->GetParsedID();// info->GetSpawnName();
    if (!info->GetSpawnName().empty()) {
        rqobj.SpawnName = info->GetSpawnName();
    }
    rqobj.ID = info->GetID();
    rqobj.AnimOffset = info->GetAnimOffset();
    rqobj.IsVisible = info->IsVisible();
    rqobj.Position = info->GetPosition();
    rqobj.MaterialName = info->GetMaterialName();
    rqobj.Material = Gfx::Pipeline::GetInstance()->GetMaterial(info->GetMaterialName());
    rqobj.Texture = Gfx::Renderer::GetInstance()->GetTexture(info->GetTextureName());
    rqobj.TextureName = info->GetTextureName();
    if (!info->GetModelName().empty()) {
        for (int i = 0; i < MAX_FRAMES; i++) {
        rqobj.Model[i] = Gfx::Model::GetInstance()->GetModel(info->GetModelName());
        }
        rqobj.HasAnimation = info->HasAnimations();
    }
    else {
        rqobj.Mesh = Gfx::Mesh::GetInstance()->GetMesh(info->GetTextureName());
    }
    return rqobj;
}

Modules::Base::Base(Keeper::Base* objects)
: GameObjects(objects)
{

}


Modules::Module::Module(Modules::Info info)
{
    BindlessType = info.BindlessType;
    IAttachments = info.IAttachments;
}
