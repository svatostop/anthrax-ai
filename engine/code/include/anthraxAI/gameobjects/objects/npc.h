#pragma once

#include "anthraxAI/gameobjects/gameobjects.h"
#include "anthraxAI/gameobjects/objects/gizmo.h"
#include <algorithm>
#include <cstdint>

namespace Keeper
{
  class Npc : public Objects
  {
        public:
            Npc() {}
            Npc(const Info& info);

            ~Npc() {}

            Type GetType() const override { return ObjectType; }
            void SetSelected(bool id) override { Selected = id; }
            void SetVisible(bool vis) override { Visible = vis; }
            bool IsVisible() const override{ return Visible; }
            void Update() override;
            void PrintInfo() override;

           const glm::vec3& GetRotation() const override { return rotation; }
           void SetRotation(const glm::vec3& v) override { rotation = v; }
    
           bool GetReflection() const override { return reflection; }
           void SetReflection(bool refl) override { reflection = refl; }
           float GetScale() const override { return scale; }
           void SetScale(float v) override { scale = v; }
            bool HasAnimations() const override { return IsAnimated; }
            void SetGizmo(Keeper::Objects* gizmo) override { GizmoHandle = reinterpret_cast<Keeper::Gizmo*>(gizmo); }

            void SetTextureName(const std::string& str) override { TextureName = str; }

            Keeper::Objects* GetGizmo() const override { return  reinterpret_cast<Keeper::Objects*>(GizmoHandle);}
            Vector3<float> GetPosition() const override { return Position; }
            const std::string& GetModelName() const override { return ModelName; }
            const std::string& GetTextureName() const override { return TextureName; }
            const std::string& GetMaterialName() const override { return MaterialName; }
            const std::string& GetFragmentName() const override { return Fragment; }
            const std::string& GetVertexName() const override { return Vertex; }
            const std::string& GetSpawnName() const override { return SpawnName; }
            const uint32_t GetInstanceCount() const override { return InstanceCount; }
            const bool GetSpawn() const override { return Spawn;}
            const std::string& GetParsedID() const override { return ParsedID; }
            const std::vector<std::string>& GetAnimations() const override { return Animations; }
        
            float GetAnimOffset() const override { return AnimOffset; }
        private:
            enum class MoveAxis {
                X= 0,
                Y,
                Z        
            };
            void move(float& val);

            Keeper::Type ObjectType = Type::NPC;

            Vector3<float> Position;
            Vector3<float> MoveDirs;
            
            float AnimOffset = 1.0;
    bool ResetMouse = false;
            glm::vec3 rotation = glm::vec3(0);        
            float scale = 1.0f;
            std::string ParsedID = "";
            std::string SpawnName = "";
            std::string Vertex;
            std::string Fragment;
            std::string TextureName;
            std::string MaterialName;
            std::string ModelName;
            bool Visible = true;
            bool Spawn = false;
            bool Selected = false;
            bool Move = false;
            bool IsAnimated = false;
            bool reflection = false;
            
            uint32_t InstanceCount = 1;
   bool reset = true;
            std::vector<std::string> Animations;
    
            Keeper::Gizmo* GizmoHandle = nullptr;
            
            MoveAxis axis = MoveAxis::X;
            float step = 0.008f;
            float min_distance = 0.0;
            float cur_distance = 0.0;
            float max_distance = 2.0;
            float start_distance = 0.0;
  };
}
