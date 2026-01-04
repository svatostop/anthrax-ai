#pragma once

#include "anthraxAI/core/animator.h"
#include "anthraxAI/gamemodules/modules.h"
#include "anthraxAI/gameobjects/gameobjects.h"
#include "anthraxAI/gfx/model.h"
#include "anthraxAI/gfx/renderhelpers.h"
#include "anthraxAI/utils/mathdefines.h"
#include "anthraxAI/utils/parser.h"
#include "anthraxAI/utils/thread.h"

#include "anthraxAI/gameobjects/gameobjects.h"
#include "anthraxAI/gameobjects/objects/camera.h"

#include <atomic>
#include <cstdint>
#include <string>
#include <vulkan/vulkan_core.h>
#include "anthraxAI/utils/tracy.h"

// #define VISIBILITY_COMPUTE 
#define COMPUTE_MTX
#define DRAW_INDIRECT
namespace Core
{
    class Scene : public Utils::Singleton<Scene>
    {
        public:
            ~Scene() { if (GameObjects) delete GameObjects; if (GameModules) delete GameModules; }
            void Init();
            void InitModules();

            void Loop();
        
            void ExportScene();
            void ExportObjectInfo(const Keeper::Objects* obj);
            void RenderScene(bool playmode);
            int GetAnimationIndexSize() { if (GameModules) { return GameModules->GetAnimationIndexSize(); } return 0;}
            int GetAnimationIndex(const std::string& id) { if (GameModules) { return GameModules->GetAnimationIndex(id); } return 0; }
            bool HasAnimator() { if (GameModules) { return GameModules->HasAnimator(); } return false; }
            const Core::aiSceneInfo& GetInfo(Gfx::ModelInfo* info, const std::string& id, float offset) {  return GameModules->GetInfo(info, id, offset);   }
            void ReloadAnimation(const std::string& id, const std::string& s) { if (GameModules) { return GameModules->ReloadAnimation(id, s); }}
            
            void SetCurrentSceneForUpdate(const std::string& name) { CurrentSceneForUpdate = name; }
            void SetCurrentScene(const std::string& str);
            Modules::ScenesMap& GetScenes() { return GameModules->GetSceneModules(); }
            Modules::Module& GetModule(const std::string& name) const { return GameModules->Get(name); }        

            Keeper::Camera& GetCamera() { return *EditorCamera; }
            const Keeper::Base* GetGameObjects() const { return GameObjects; }
            void SetNewObjectType(const std::string& str) { GameObjects->NewObjectInfo.Type = str; }
            const std::string& GetNewObjectType() { return GameObjects->NewObjectInfo.Type; }
            void SetNewObjectLightType(const std::string& str) { GameObjects->NewObjectInfo.LightType = str; }
            const std::string& GetNewObjectLightType() { return GameObjects->NewObjectInfo.LightType; }
            void SetNewObjectParsedID(const std::string& str) { GameObjects->NewObjectInfo.ParsedID = str; }
            const std::string& GetNewObjectParsedID() { return GameObjects->NewObjectInfo.ParsedID; }
            void SetNewObjectColor(glm::vec3 position) { GameObjects->NewObjectInfo.Color = position; }
            glm::vec3 GetNewObjectColor() { return GameObjects->NewObjectInfo.Color.convert(); }
            void SetNewObjectPosition(glm::vec3 position) { GameObjects->NewObjectInfo.Position = position; }
            glm::vec3 GetNewObjectPosition() { return GameObjects->NewObjectInfo.Position.convert(); }
            void SetNewObjectTexture(const std::string& str) { GameObjects->NewObjectInfo.Texture = str; }
            const std::string& GetNewObjectTexture() { return GameObjects->NewObjectInfo.Texture; }
            void SetNewObjectMaterial(const std::string& str) { GameObjects->NewObjectInfo.Material = str; }
            const std::string& GetNewObjectMaterial() { return GameObjects->NewObjectInfo.Material; }
            void SetNewObjectModel(const std::string& str) { GameObjects->NewObjectInfo.Model = str; }
            const std::string& GetNewObjectModel() { return GameObjects->NewObjectInfo.Model; }
            void ClearNewObjectInfo();
            void DeleteSelectedObject();
            void SaveObject();// { GameObjects->Create<Keeper::Npc>(new Keeper::Npc(GameObjects->NewObjectInfo)); }        

            void ReloadResources();
            void ParseSceneNames();
            const std::vector<std::string>& GetSceneNames() const { return SceneNames; }
            void NewScene();

            void KeepEditor(bool keep) { HasEditor = keep; }
            bool GetKeepEditor() { return HasEditor; }
            const std::string& GetCurrentScene() const { return CurrentScene; }
            const std::string& GetCurrentSceneForUpdate() const { return CurrentSceneForUpdate; }
            void SetSelectedID(uint32_t id) { GameObjects->SetSelectedID(id); }
            uint32_t GetSelectedID() { return GameObjects->GetSelectedID(); }

            const glm::mat4& GetGlobalTransform() const { return GameModules->GetGlobalTransform(); }
            bool HasGameModules() const { return GameModules; }
            bool RenderPassed = false;

            uint32_t GetCubemapBind(uint32_t ind) { return GameModules->GetCubemapBind(ind);}

            void SetShadows(bool shadows) { HasFrameShadows = shadows; }
            bool GetShadows() { return HasFrameShadows; }

            void SetParticles(bool particle) { HasCompute = particle; }
            bool GetParticles() { return HasCompute; }

            size_t GetSceneVertSize() { return GameModules->GetSceneVertSize(); }
        private:
            void PopulateModules();

            void LoadScene(const std::string& filename);
            void Render(Modules::Module& module);
            void Compute(Modules::Module& module, uint32_t work_groups, uint32_t work_groups2 = 1);
            void RenderThreaded(Modules::Module& module);

            Keeper::Base* GameObjects = nullptr;
            Modules::Base* GameModules = nullptr;

            std::string CurrentSceneForUpdate;
            std::string CurrentScene = "intro";
            std::vector<Keeper::Info> ParsedSceneInfo;
            bool IsNewScene = false;
            std::vector<std::string> SceneNames;

            Keeper::Camera* EditorCamera;

            Utils::Parser Parse;
            VkPipeline ThreadedPipeline;
            std::vector<VkCommandBuffer> sec_cmds;
            bool HasEditor = true;
            bool HasGBuffer = false;
            bool HasCompute = false;
            bool HasFrameGizmo = false;
            bool HasFrameOutline = false;
            bool HasFrameGrid = false;
            bool HasFrameShadows = false;
    };

}
