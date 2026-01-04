#pragma once

#include "anthraxAI/gfx/model.h"
#include "anthraxAI/gfx/renderhelpers.h"
#include "glm/fwd.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/anim.h>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Core
{
  struct AnimationData {

  	std::string CurrentPath;
  	std::vector<std::string> Paths;
  	int PathIndex = 0;
  	int SceneInd = 0;
    int animation_array_ind = 0;
  };
    
    
    struct NodeRoots {
        std::string Name;
        glm::mat4 Transform;
        int ind;
        int ChildrenNum;
        std::vector<NodeRoots> Children;

        NodeRoots() {}
    };
    struct NodeAnim {
        std::string NodeName;
        uint32_t NumPositionsKeys = 0;
        std::vector<glm::vec3> PositionKeys;
        std::vector<float> PositionTime;
        uint32_t NumRotationKeys = 0;
        std::vector<glm::quat> RotationKeys;
        std::vector<float> RotationTime;
        uint32_t NumScalingKeys = 0;
        std::vector<glm::vec3> ScalingKeys;
        std::vector<float> ScalingTime;

        NodeAnim() {}
    };

    struct aiSceneInfo {
        float TicksPerSecond;
        float Duration;
        NodeRoots RootNode;
        // std::vector<Core::NodeRoots> nodes;
        // Core::NodeRoots nodes[108];
        // std::vector<NodeAnim> AnimNodes;
        NodeAnim AnimNodes[108];
    //     //-------------
            //glm::mat4 animrot[108];
        Gfx::AnimMatricies matricies;    
        
        Gfx::AnimFloats floats;
        
    std::string Names[108];
        int animsize;
        int rootssize = 0;
        float timeticks;
        
        aiSceneInfo(){};
    };

   
  class AnimatorBase {

    public:
        AnimatorBase() {};

  	    void Init();
        void Update(Gfx::RenderObject& object);

        Core::aiSceneInfo& Update2(Gfx::ModelInfo* model, const std::string& id, float offset);

        bool HasAnimation(const std::string& id) {
    		return Animations.find(id) != Animations.end();
    	}

    void Reload(const std::string& id, const std::string& name) {
            Animations[id].CurrentPath = name;
            auto it = std::find(AnimationMap.begin(), AnimationMap.end(), "./models/" + name);
            std::size_t index = std::distance(AnimationMap.begin(), it);
            Animations[id].SceneInd = index;
            printf("INDEX %d\n", index);
    	}
        
    const glm::mat4& GetGlobalTransform() const { return GlobalInverse; }

    int get_animation_array_index(const std::string& id) { if (Animations.find(id) != Animations.end()) { return Animations[id].animation_array_ind; } return -1; }
    int get_animation_array_index_size() { return Animations.size(); }
    private:
    	Assimp::Importer Importer;
        std::vector<aiSceneInfo> Scenes;

        glm::mat4 GlobalInverse;
    	std::map<std::string, AnimationData> Animations;
        std::vector<std::string> AnimationMap;


        aiSceneInfo ConvertAi(const aiScene* scene);

    	glm::mat4 InterpolatePos(glm::vec3 out, float timeticks, const NodeAnim& animnode);
    	// u_int FindPos(float timeticks, const NodeAnim& animnode);
    	glm::mat4 InterpolateRot(glm::quat out, float timeticks, const NodeAnim& animnode);
    	// u_int FindRot(float timeticks, const NodeAnim& animnode);
    	glm::mat4 InterpolateScale(glm::vec3 out, float timeticks, const NodeAnim& nodeanim );
    	// u_int FindScale(float timeticks, const NodeAnim& animnode);

    	void InterpolateRot(float timeticks, const NodeAnim& animnode, aiSceneInfo& scene, int ind);
        void InterpolatePos(float timeticks, const NodeAnim& animnode, aiSceneInfo& scene, int ind);
    	u_int FindPos(float timeticks, const NodeAnim& animnode);
    	u_int FindRot(float timeticks, const NodeAnim& animnode);
    	void InterpolateScale(float timeticks, const NodeAnim& animnode, aiSceneInfo& scene, int ind);
    	u_int FindScale(float timeticks, const NodeAnim& animnode);

    
        void ReadNodes(aiSceneInfo& scene, Core::NodeRoots& node,  int ind);
    	const Core::NodeAnim& FindAnim(const aiSceneInfo& scene, const std::string nodename);
    	int FindAnimInt(const aiSceneInfo& scene, const std::string& nodename);
    	void ReadNodeHierarchy(Gfx::ModelInfo* model, int animid, const Core::aiSceneInfo& scene, const NodeRoots& node, float timetick, const glm::mat4 parenttransform);
    	void ReadNodeHierarchy3(int i, Gfx::ModelInfo* model,  Core::aiSceneInfo& scene);
    	void Hierarchy(Gfx::ModelInfo* model, Core::aiSceneInfo& scene, int i, float timetick, glm::mat4 parenttransform, glm::mat4 parenttransforms[200]);
        void GetBonesTransform(Gfx::ModelInfo* model, int animid, float time);
        Core::aiSceneInfo& GetBonesTransform2(Gfx::ModelInfo* model, const std::string& animid, float time);
        
        void GetNodeChildren(const aiNode* node, Core::NodeRoots& info);
           Core::NodeAnim EMPTY_ANIM_NODE;
  };
}
