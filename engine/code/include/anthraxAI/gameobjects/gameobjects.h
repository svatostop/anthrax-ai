#pragma once

#include <atomic>
#include <cstdint>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <vector>
#include <map>

#include "anthraxAI/utils/defines.h"
#include "anthraxAI/utils/mathdefines.h"

namespace Keeper {

    struct Info {
        Vector3<float> Position;
        Vector3<float> Rotation = {0,0,0};
        Vector3<float> Color;
        Vector3<float> Offset;
        std::string LightType;
        std::string Material;
        std::string Vertex;
        std::string Fragment;
        std::vector<std::string> Textures;
        std::string Texture;
        std::string Model;
        std::string Mesh;
        std::vector<std::string> Animations;
        std::string ParsedID = "";
        std::string SpawnName = "";
        bool Spawn = false;
        bool IsModel = false;
        bool IsLight = false;
        bool VertexBase = false;

        float  Scale = 1.0;
        float AnimOffset = 1.0;
        // used in UI object creation only
        std::string Type;
    };

    enum Type {
        CAMERA = 0,
        SPRITE,
        NPC,
        GIZMO,
        TEST,
        LIGHT,
        SIZE
    };

    enum Infos {
        INFO_INTRO = 0,
        INFO_GRID,
        INFO_MASK,
        INFO_OUTLINE,
        INFO_GBUFFER,
        INFO_SHADOWS,
        INFO_LIGHTING,
        INFO_SKYBOX,
        INFO_PARTICLES,
        INFO_COMPUTE_MTX,
        INFO_VISIBILITY_COMPUTE,
        INFO_COMPUTE_SKINNING,
        INFO_SIZE
    };

    class Objects
    {
        public:
            Objects() { UniqueID = IDCounter; IDCounter++; }
            Objects(const Info& info) { UniqueID = IDCounter; IDCounter++; }
            Objects(const Info& info, int axis) { UniqueID = IDCounter; IDCounter++; }
            virtual ~Objects() {}

            virtual Type GetType() const { return SIZE; }
            virtual int GetAxis() const { return -1; }
            virtual uint32_t CameraType() const { return -1; }
            virtual uint32_t LightType() const { return -1; }

            virtual const std::string& GetModelName() const { return EMPTY_STRING; }
            virtual const std::string& GetTextureName() const { return EMPTY_STRING; }
            virtual const std::string& GetFragmentName() const { return EMPTY_STRING; }
            virtual const std::string& GetVertexName() const { return EMPTY_STRING; }
            virtual const std::string& GetMaterialName() const { return EMPTY_STRING; }
            virtual Vector3<float> GetPosition() const { return {0.0f, 0.0f, 0.0f}; }
            virtual Vector3<float> GetColor() const { return {0.0f, 0.0f, 0.0f}; }
            virtual float GetRadius() const { return 0.0f; }
            virtual const std::vector<std::string>& GetAnimations() const { return EmptyAnimations; }

            virtual const glm::vec3& GetRotation() const { return EmptyVec3; }
            virtual float GetScale() const { return 1.0f; }
            virtual Keeper::Objects* GetHandle() const { return nullptr; }
            virtual Keeper::Objects* GetGizmo() const { return  nullptr;}

            virtual void SetTextureName(const std::string& str) {}
            virtual void SetSelected(bool id) { }
            virtual void SetGizmo(Keeper::Objects* gizmo) {}
            virtual void SetPosition(const Vector3<float> pos) { }
            virtual void SetHandle(Keeper::Objects* id) { }
            virtual void SetVisible(bool vis) { }
            virtual bool IsVisible() const { return true; }
            virtual bool HasAnimations() const { return false; }

            virtual float GetAnimOffset() const { return 1.0; }
            virtual void Update() {}

            virtual void PrintInfo() {}

            virtual int GetID() const { return UniqueID; }
            virtual const std::string& GetParsedID() const { return ParsedID; }
            virtual void ResetCounterID() { IDCounter = 1; }

           virtual const std::string& GetSpawnName() const { return EMPTY_STRING; }
           virtual const uint32_t GetInstanceCount() const { return 1; }
           virtual const bool GetSpawn() const { return false;}
            
           virtual void SetRotation(const glm::vec3& v) { }
           virtual void SetScale(float v) { }
        private:
            std::string ParsedID = "";
            std::string EMPTY_STRING = "";
            std::vector<std::string> EmptyAnimations;
            glm::vec3 EmptyVec3 = glm::vec3(0);
            int UniqueID = 1;
            inline static std::atomic_int IDCounter = 1;
    };

    typedef std::map<int, std::vector<Objects*>> GameObjectsMap;

    class Base
    {
        public:
            Base();
            ~Base();
            void CleanIfNot(Keeper::Type type, bool resetID = false);
            template<typename T>
            void Create(T* type) { ASSERT(type->GetType() == SIZE, "Keeper::Base::Create(): Error: child has no GetType() defined"); ObjectsList[type->GetType()].push_back(type); }

            bool IsValid(Type type) { if (!ObjectsList[type].empty()) { return true; } return false; }
            std::vector<Objects*>& Get(Type type) {  return ObjectsList[type]; }

            void Create(const std::vector<Info>& info);
            void Update();

            std::vector<Objects*> GetObjects(Type type) { return (ObjectsList[type]); }
            const GameObjectsMap& GetObjects() const { return ObjectsList; }

            Info& GetGizmoInfo(int type) { return GizmoInfo[type]; } ;

            void SetSelectedID(uint32_t id) { SelectedID = id; }
            uint32_t GetSelectedID() { return SelectedID; }

            size_t GetObjectsSize() const;
            void UpdateObjectNames();
            void VerifyNewObject();
            bool EraseSelected();

            Keeper::Objects* GetNotConstObject(Keeper::Type type, int id);
            Keeper::Objects* GetNotConstObject(Keeper::Type type, const std::string& str);
            const Keeper::Objects* GetObject(Keeper::Type type, int id) const;
            Keeper::Objects* GetLast(Keeper::Type type) { return ObjectsList[type].at(ObjectsList[type].size() - 1); }
            const std::vector<std::string>& GetObjectNames() const { return ObjectNames; }
            const std::vector<std::string>& GetObjectTypes() const { return ObjectTypes; }

            Info GetInfo(Infos info) const { return DefaultObjects[info]; }

            bool Find(Keeper::Type type) const;
            Keeper::Info NewObjectInfo;
            void ClearNewObjectInfo();
        private:
            void SpawnObjects(const Info& info);

            GameObjectsMap ObjectsList;
            Info GizmoInfo[3];
            Info DefaultObjects[INFO_SIZE];
            uint32_t SelectedID = 0;
            
            std::vector<std::string> ObjectNames;
            std::vector<std::string> ObjectTypes;
    };
}
