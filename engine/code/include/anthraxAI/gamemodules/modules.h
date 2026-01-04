#pragma once

#include <atomic>
#include <cstdint>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <vector>
#include <map>

#include "anthraxAI/core/animator.h"
#include "anthraxAI/utils/defines.h"
#include "anthraxAI/utils/mathdefines.h"
#include "anthraxAI/gfx/renderhelpers.h"
#include "anthraxAI/gameobjects/gameobjects.h"
#include "anthraxAI/core/imguihelper.h"

namespace Modules
{
    struct Info {
        int ColorID;
        int DepthID;
        Gfx::InputAttachments IAttachments;
        Gfx::BindlessDataType BindlessType;
    };

    enum Update {
        RESOURCES,
        MATERIALS,
        RQ,
        TEXTURE_UI_MANAGER,
        SAMPLERS,
    };
    
    enum QueueType {
        RQ_GENERAL,
        RQ_LIGHT,
        RQ_SKINNED,
    };
    typedef std::vector<Gfx::RenderObject> RenderQueueVec;
    typedef std::map<QueueType, std::vector<Gfx::RenderObject>> RenderQueueMap;
    class Module
    {
        public:
            Module() {}
            Module(Info info);

            RenderQueueVec& GetRenderQueue(QueueType type) { return RenderQueue[type]; }
            RenderQueueMap& GetRenderQueueMap() { return RenderQueue; }
            Gfx::BindlessDataType GetBindlessType() const { return BindlessType; }
            Gfx::InputAttachments GetIAttachments() const { return IAttachments; }

            bool GetCameraBuffer() const { return HasCameraBuffer; }
            bool GetStorageBuffer() const { return HasStorageBuffer; }
            bool GetGizmo() const { return HasGizmo; }
            const std::string& GetTag() const { return Tag; }

            void SetTag(std::string tag) { Tag = tag; }
            void SetTexture(bool set) { HasTexture = set; }
            void SetStorageBuffer(bool set) { HasStorageBuffer = set; }
            void SetCameraBuffer(bool set) { HasCameraBuffer = set; }
            void SetGizmo(bool gizmo) { HasGizmo = gizmo; }
            void AddRQ(QueueType type, Gfx::RenderObject obj) { RenderQueue[type].push_back(obj); }

            void SetRenderQueue(QueueType type, std::vector<Gfx::RenderObject>& rq) { RenderQueue[type] = rq; }
            void SetSkinnginRenderQueue(std::vector<Gfx::RenderObject>& rq) { SkinningRqHelper = rq; }
             std::vector<Gfx::RenderObject>& GetSkinningRQ()  { return SkinningRqHelper; } 
        private:
            std::string Tag;
            Gfx::InputAttachments IAttachments;
            RenderQueueMap RenderQueue;
            Gfx::BindlessDataType BindlessType;
            RenderQueueVec SkinningRqHelper;

            int ColorID;
            int DepthID;

            bool HasCameraBuffer;
            bool HasStorageBuffer;
            bool HasTexture;
            bool HasGizmo;


    };
    typedef std::unordered_map<std::string, Module> ScenesMap;
    class Base
    {
        public:
            Base(Keeper::Base* objects);
            ~Base() { if (Animator) delete Animator; }
            void Clear();

            void Update(uint32_t update_type, bool force_update = false);
            
            void Insert(const Keeper::Objects* obj);
            void Populate(const std::string& key, Info scene, std::function<bool(Keeper::Type)> skip_type);
            void Populate(const std::string& key, Info scene, Keeper::Info info);
            void Populate(const std::string& key, Info scene, std::vector<Keeper::Objects*> vec);

            Module& Get(const std::string& key) { return SceneModules[key]; }
            bool Find(const std::string& key) const { return SceneModules.find(key) != SceneModules.end(); }        

            bool HasFrameOutline() const { return HasOutline; }
            void ReloadAnimation(const std::string& id, const std::string& s) { if (Animator) { Animator->Reload(id, s); } }
// #ifndef COMPUTE_MTX
            // bool HasAnimation(uint32_t id) { if (Animator) { return Animator->HasAnimation(id); } return false; }
// #endif
            int GetAnimationIndex(const std::string& id) { return Animator->get_animation_array_index(id); }
            int GetAnimationIndexSize() { return Animator->get_animation_array_index_size(); }
            const Core::aiSceneInfo& GetInfo(Gfx::ModelInfo* info, const std::string& id, float offset) {  return Animator->Update2(info, id, offset);   }
            void SetRenderQueue(QueueType type, const std::string& key, RenderQueueVec& rq) { SceneModules[key].SetRenderQueue(type, rq); }
            void SetSkinnginRenderQueue(const std::string& key, RenderQueueVec& rq) { SceneModules[key].SetSkinnginRenderQueue( rq); }

            void RestartAnimator();
            bool HasAnimator() const { return Animator; }
            const glm::mat4& GetGlobalTransform() const { return Animator->GetGlobalTransform(); }

            ScenesMap& GetSceneModules() { return SceneModules; }
            void SetCurrentScene(const std::string& str) { CurrentScene = str; }
            
            void EraseSelected();

            uint32_t GetCubemapBind(uint32_t ind) { return CubemapBind[ind]; }
            void SetCubemapTexture(const std::string& s) { CubemapTexture = s; }
            const std::string& GetCubemapTexture() const { return CubemapTexture; }
            size_t GetSceneVertSize() { return vertex_count; }

            const std::unordered_map<std::string, int>& get_mapped_spawn_vertecies();
        private:
            uint32_t CubemapBind[MAX_FRAMES];
            bool HasOutline = false;
            std::string CurrentScene;
            ScenesMap SceneModules;
            Keeper::Base* GameObjects = nullptr;
            Core::AnimatorBase* Animator = nullptr;

            std::string CubemapTexture;
            
            bool was_vertecies_count = 0;
            int vertex_count = 0;
            std::unordered_map<std::string, int> mapped_spawn_vertecies;
            Gfx::RenderObject LoadResources(const Keeper::Objects* info);
            void UpdateResources();
            void UpdateResource(Modules::Module& module, Gfx::RenderObject& obj);
            void UpdateMaterials();
            bool UpdateTexture(const std::string& str, Core::ImGuiHelper::TextureForUpdate upd);
            void UpdateTextureUIManager();
            void UpdateSamplers();
            void UpdateRQ();
            void ThreadedRQ(int i, Keeper::Objects* info);
    };

}
