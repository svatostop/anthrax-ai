#include "anthraxAI/core/animator.h"
#include "anthraxAI/gfx/model.h"
#include "anthraxAI/utils/debug.h"
#include "anthraxAI/engine.h"
#include "anthraxAI/utils/defines.h"
#include "assimp/anim.h"
#include "assimp/scene.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

void Core::AnimatorBase::Update(Gfx::RenderObject& object)
{
#ifdef TRACY
    ZoneScopedN("Core::AnimatorBase::Update");
#endif
    if (Animations.empty()) {
        return;
    }

    float timesec = (float)((double)Engine::GetInstance()->GetTime() - (double)Utils::Debug::GetInstance()->AnimStartMs) / 1000.0f;
    uint32_t frame = Gfx::Renderer::GetInstance()->GetFrameInd();
    GetBonesTransform(object.Model[frame], object.ID, timesec);
}
Core::aiSceneInfo& Core::AnimatorBase::Update2(Gfx::ModelInfo* model, const std::string& id, float offset)
{
#ifdef TRACY
    ZoneScopedN("Core::AnimatorBase::Update2");
#endif
    float timesec = (float)((double)Engine::GetInstance()->GetTime() - (double)Utils::Debug::GetInstance()->AnimStartMs ) / 1000.0f;
    timesec *= offset;
    return GetBonesTransform2(model, id, timesec);
}

const Core::NodeAnim& Core::AnimatorBase::FindAnim(const aiSceneInfo& scene, const std::string nodename)
{
    for (int i = 0; i < scene.animsize; i++) {
        //const aiNodeAnim* node = anim->mChannels[i];
        const NodeAnim& node = scene.AnimNodes[i];
        if (node.NodeName == nodename) {
            return node;
        }
    }
    return EMPTY_ANIM_NODE;
}
int Core::AnimatorBase::FindAnimInt(const aiSceneInfo& scene, const std::string& nodename)
{
#ifdef TRACY
    ZoneScopedN("Core::AnimatorBase::FindAnimInt");
#endif
    
    if (!nodename.empty()) {
        int i = 0;
    //for (int i = 0; i < scene.animsize; i++) {
    for (const NodeAnim& node : scene.AnimNodes) {
        //const aiNodeAnim* node = anim->mChannels[i];
        // const NodeAnim& node = scene.AnimNodes[i];
        if (node.NodeName.empty()) continue;
        if (node.NodeName == nodename) {
            return i;
        }
        i++;
    //}
    }
    }
    return -1;
}

// void Core::AnimatorBase::ClearNode(Core::NodeRoots* info)
// {
//     for (int i = 0; i < info->ChildrenNum; i++) {
//         ClearNode(info->Children[i]);
//     }
//     delete info;
// }
// void Core::AnimatorBase::ClearNodeChildren()
// {
//     for (int i = 0; i < Scenes.size(); i++) {
//         ClearNode(Scenes[i].RootNode);
//     }
// }
//
void Core::AnimatorBase::GetNodeChildren(const aiNode* node, Core::NodeRoots& info)
{
    info.Name = node->mName.C_Str();
    info.Transform = Gfx::mat2glm(node->mTransformation);
    info.ChildrenNum = node->mNumChildren;
    info.Children.reserve(info.ChildrenNum);
    // if (v.empty()) { 
    // }
    for (int i = 0; i < node->mNumChildren; i++) {
        NodeRoots nodetmp ;
        GetNodeChildren(node->mChildren[i], nodetmp);
        info.Children.emplace_back(nodetmp);
         // v.push_back(nodetmp);
    }
    // v.push_back(info);
}

Core::aiSceneInfo Core::AnimatorBase::ConvertAi(const aiScene* scene)
{
    Core::aiSceneInfo info;

    info.TicksPerSecond = scene->mAnimations[0]->mTicksPerSecond;
    info.Duration = scene->mAnimations[0]->mDuration;


    Core::NodeRoots root;
    GetNodeChildren(scene->mRootNode, root);
    info.RootNode = root;
printf("anim node channels: %d\n", scene->mAnimations[0]->mNumChannels );    
    ASSERT(scene->mAnimations[0]->mNumChannels > 108, "Anim Nodes size is > 100!");
    info.animsize = scene->mAnimations[0]->mNumChannels;
    // info.AnimNodes.resize(scene->mAnimations[0]->mNumChannels);

    for (int i = 0; i < scene->mAnimations[0]->mNumChannels; i++) {
        const aiNodeAnim* node = scene->mAnimations[0]->mChannels[i];

        NodeAnim& anim = info.AnimNodes[i];
        anim.NodeName = node->mNodeName.data;// C_Str();
        anim.NumPositionsKeys = node->mNumPositionKeys;
        anim.NumRotationKeys = node->mNumRotationKeys;
        anim.NumScalingKeys = node->mNumScalingKeys;

        anim.PositionKeys.reserve(anim.NumPositionsKeys);
        anim.PositionTime.reserve(anim.NumPositionsKeys);
        for (int posi = 0; posi < node->mNumPositionKeys; posi++) {
            anim.PositionKeys.emplace_back(Gfx::vec2glm(node->mPositionKeys[posi].mValue));
            anim.PositionTime.emplace_back(node->mPositionKeys[posi].mTime);
        }
        anim.RotationKeys.reserve(anim.NumRotationKeys);
        anim.RotationTime.reserve(anim.NumRotationKeys);
        for (int posi = 0; posi < node->mNumRotationKeys; posi++) {
            anim.RotationKeys.emplace_back(Gfx::quat2glm(node->mRotationKeys[posi].mValue));
            anim.RotationTime.emplace_back(node->mRotationKeys[posi].mTime);
        }
        anim.ScalingKeys.reserve(anim.NumScalingKeys);
        anim.ScalingTime.reserve(anim.NumScalingKeys);
        for (int posi = 0; posi < node->mNumScalingKeys; posi++) {
            anim.ScalingKeys.emplace_back(Gfx::vec2glm(node->mScalingKeys[posi].mValue));
            anim.ScalingTime.emplace_back(node->mScalingKeys[posi].mTime);
        }
        //info.AnimNodes.emplace_back(anim);
    }

    return info;
}


void Core::AnimatorBase::Init()
{
    int animation_ind = 0;
    Core::Scene* scene = Core::Scene::GetInstance();
    for (auto& it : scene->GetGameObjects()->GetObjects()) {
        for (Keeper::Objects* info : it.second) {
            if (info->GetModelName().empty() || !info->HasAnimations()) continue;
            // AnimationData& data =  Animations[info->GetID()];
            std::string key = info->GetParsedID();
            if (!info->GetSpawnName().empty()) {
                key = info->GetSpawnName();
            }
            if (Animations.find(key) != Animations.end()) continue;;

            AnimationData& data =  Animations[key];
            data.Paths.reserve(info->GetAnimations().size());
            data.SceneInd = 0;
            data.animation_array_ind = animation_ind;
            for (const std::string& animpath : info->GetAnimations()) {
                data.Paths.push_back("./models/" + animpath);

                auto it = std::find(AnimationMap.begin(), AnimationMap.end(), "./models/" + animpath);

                std::size_t index = std::distance(std::begin(AnimationMap), it);
                if (it == AnimationMap.end()) {
                    const aiScene* sceneai = Importer.ReadFile("./models/" + animpath, IMPORT_PROPS);
                    aiSceneInfo scene = ConvertAi(sceneai);
      
                    Scenes.push_back(scene);
                    AnimationMap.push_back(std::string("./models/" + animpath));
                    data.SceneInd = AnimationMap.size() - 1;
                    Importer.FreeScene();
                }
            }
            data.CurrentPath = data.Paths[0];
            data.PathIndex = 0;
            animation_ind++;
        }
    }
    GlobalInverse = glm::inverse(glm::mat4(1.0, 0.0, 0.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0));
    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
    GlobalInverse *= rot ;//*  glm::scale(glm::mat4(1.0f), glm::vec3(0.03));
}

void Core::AnimatorBase::Hierarchy(Gfx::ModelInfo* model, Core::aiSceneInfo& scene, int i, float timetick, glm::mat4 parenttransform, glm::mat4 parenttransforms[200])
{
    glm::mat4 nodetransf = scene.matricies.nodeTransform[i];
    glm::mat4 globaltransf; 
    if (scene.floats.nodeAnimInd[i] != -1) { // scene.floats.animisempty[scene.floats.nodeAnimInd[i]] == 0) {
                {
#ifdef TRACY
        ZoneNamedN(ZoneAnim5, "AnimatorBase::ReadNodeHierarchy::Mult-scale-rot-transl", true);
#endif
        // nodetransf = scene.animnodes[scene.noderoots[i].AnimInd].pos * scene.animnodes[scene.noderoots[i].AnimInd].rot *  scene.animnodes[scene.noderoots[i].AnimInd].scale;
        }
    }
    {
#ifdef TRACY
        ZoneNamedN(ZoneAnim6, "AnimatorBase::ReadNodeHierarchy::Final-transform", true);
#endif
    globaltransf = ( scene.floats.nodeIndex[i] == 0 ? parenttransform : parenttransforms[ scene.floats.nodeIndex[i] - 1]) * nodetransf;
//         if (node.ind == 0) {
// parenttransforms[node.ind] = parenttransform; 
//         }
//         else {
    parenttransforms[ scene.floats.nodeIndex[i]] = globaltransf;
        // }
    if (scene.floats.nodeBoneInd[i] != -1) {
        int boneind = scene.floats.nodeBoneInd[i];
        model->Bones.Info[boneind].FinTransform = GlobalInverse * globaltransf * scene.matricies.nodeOffset[i];
    }
    }

}
void Core::AnimatorBase::ReadNodeHierarchy3(int i, Gfx::ModelInfo* model,  Core::aiSceneInfo& scene)
{
    std::string& nodename = scene.Names[i];
    int animind = FindAnimInt(scene, nodename);
    scene.floats.nodeAnimInd[i] = animind;
    // scene.floats.animisempty[animind] = 1;
// printf("animind: %d\n", animind);
    if (animind != -1) {
        // scene.floats.animisempty[animind] = 0;
        {
#ifdef TRACY
        ZoneNamedN(ZoneAnim2, "AnimatorBase::ReadNodeHierarchy::Scaling", true);
#endif
        // glm::vec3 scaling;
        // scene.animnodes[animind].scale = InterpolateScale(scaling, scene.timeticks, scene.AnimNodes[scene.noderoots[i].AnimInd]);
        InterpolateScale(scene.timeticks, scene.AnimNodes[scene.floats.nodeAnimInd[i]], scene, animind);
        }
        {
#ifdef TRACY
        ZoneNamedN(ZoneAnim3, "AnimatorBase::ReadNodeHierarchy::Rotating", true);
#endif
        InterpolateRot(scene.timeticks, scene.AnimNodes[scene.floats.nodeAnimInd[i]], scene, animind);
        }
        {
#ifdef TRACY
        ZoneNamedN(ZoneAnim4, "AnimatorBase::ReadNodeHierarchy::Trtanslating", true);
#endif
        // glm::vec3 Translation;
        // scene.animnodes[animind].pos = InterpolatePos(Translation, scene.timeticks, scene.AnimNodes[scene.noderoots[i].AnimInd]);
        InterpolatePos(scene.timeticks, scene.AnimNodes[scene.floats.nodeAnimInd[i]], scene, animind);
        }

    }
    //scene.floats.nodesettransform[i] = 0;
    scene.floats.nodeBoneInd[i] = -1;
    const auto& it = model->Bones.BoneMap.find(nodename);
    if (it != model->Bones.BoneMap.end()) {
        // scene.floats.nodesettransform[i] = 1;
        scene.floats.nodeBoneInd[i] = it->second;
        scene.matricies.nodeOffset[i] = model->Bones.Info[it->second].Offset;
    }
}

int globalind = 0;
void Core::AnimatorBase::ReadNodes(aiSceneInfo& scene, Core::NodeRoots& node,  int ind)
{
#ifdef TRACY
    ZoneScopedN("Core::AnimatorBase::ReadNodes");
#endif
    node.ind = ind; 
    // v.push_back(node);

    scene.floats.nodeIndex[globalind] = node.ind;
    scene.matricies.nodeTransform[globalind] = node.Transform;
    scene.Names[globalind] = node.Name;
//push_back(node);;//[globalind] = node;
    globalind++;
    ind++;


    for (int i = 0; i < node.ChildrenNum; i++) {
        ReadNodes(scene, node.Children[i], ind);
    }

}
void Core::AnimatorBase::ReadNodeHierarchy(Gfx::ModelInfo* model, int animid, const Core::aiSceneInfo& scene, const Core::NodeRoots& node, float timetick, const glm::mat4 parenttransform)
{
#ifdef TRACY
    ZoneNamedN(ZoneAnim1, "AnimatorBase::ReadNodeHierarchy", true);
#endif
    std::string nodename = node.Name;

    const NodeAnim& nodeanim = FindAnim(scene, nodename);

   glm::mat4 nodetransf = node.Transform;
glm::mat4 globaltransf; 
    if (!nodeanim.NodeName.empty()) {
        glm::mat4 scm, rotm, transl;
        {
#ifdef TRACY
        ZoneNamedN(ZoneAnim2, "AnimatorBase::ReadNodeHierarchy::Scaling", true);
#endif
        glm::vec3 scaling;
        scm = InterpolateScale(scaling, timetick, nodeanim);
        }
        {
#ifdef TRACY
        ZoneNamedN(ZoneAnim3, "AnimatorBase::ReadNodeHierarchy::Rotating", true);
#endif
        glm::quat rot;
        rotm = InterpolateRot(rot, timetick, nodeanim);
        }
        {
#ifdef TRACY
        ZoneNamedN(ZoneAnim4, "AnimatorBase::ReadNodeHierarchy::Trtanslating", true);
#endif
        glm::vec3 Translation;
        transl = InterpolatePos(Translation, timetick, nodeanim);
        }
        {
#ifdef TRACY
        ZoneNamedN(ZoneAnim5, "AnimatorBase::ReadNodeHierarchy::Mult-scale-rot-transl", true);
#endif
        nodetransf = transl * rotm * scm;
        }
    }
    {
#ifdef TRACY
        ZoneNamedN(ZoneAnim6, "AnimatorBase::ReadNodeHierarchy::Final-transform", true);
#endif
    globaltransf = parenttransform * nodetransf;

    if (model->Bones.BoneMap.find(nodename) != model->Bones.BoneMap.end()) {
        int boneind = model->Bones.BoneMap[nodename];
        model->Bones.Info[boneind].FinTransform = GlobalInverse * globaltransf * model->Bones.Info[boneind].Offset;
        //model->Bones.FinTransform[boneind] = GlobalInverse * globaltransf * model->Bones.Info[boneind].Offset;

    }
    }

    for (int i = 0; i < node.ChildrenNum; i++) {
        ReadNodeHierarchy(model, animid, scene, node.Children[i], timetick, globaltransf);
    }
}

Core::aiSceneInfo& Core::AnimatorBase::GetBonesTransform2(Gfx::ModelInfo* model, const std::string& animid, float time)
{
#ifdef TRACY
    ZoneNamedN(Zone1, "Core::AnimatorBase::GetBonesTransform2", true);
#endif

    Core::AnimationData& data = Animations[animid];

    Core::aiSceneInfo& scene = Scenes[data.SceneInd];

// only aiSceneInfo ????

    float tickespersec = (float)(scene.TicksPerSecond != 0 ? scene.TicksPerSecond : 25.0f);

    float timeinticks = time  * tickespersec;
    float timeticks =  fmod(timeinticks, (float)scene.Duration);
    
    //-------2------
    globalind = 0;
   //scene.nodes.clear();
    int iii = 0;
    if (scene.rootssize == 0) {
    ReadNodes(scene, scene.RootNode, iii);
    scene.rootssize = globalind;//scene.nodes.size();
//globalind;//scene.nodes.size();
    }
    scene.timeticks = timeticks;
    // for (int i = 0; i < scene.rootssize; i++) {
    //     Gfx::NodeRootsCompute root;
    //     root.Index = scene.nodes[i].ind;
    //     root.Transform = scene.nodes[i].Transform;
    //     scene.noderoots[i] = root;
    // }
    //
// printf("----------\n");
    Thread::Pool::GetInstance()->Push({
    Thread::Task::Name::ANIM, Thread::Task::Type::EXECUTE, {}, {}, [this](Gfx::ModelInfo* model, Core::aiSceneInfo* scene) {
    for (int i = 0; i < scene->rootssize; i++) {
            ReadNodeHierarchy3(i, model, *scene);

    }
    },{},  0,  nullptr, nullptr, nullptr, model, &scene});


    // for (int i = 0; i < scene.rootssize; i++) {
    //     Hierarchy(model, scene, i, timeticks, glm::mat4(1), globaltransf);
    // }

    return scene;
}
void Core::AnimatorBase::GetBonesTransform(Gfx::ModelInfo* model, int animid, float time)
{
    #ifndef COMPUTE_MTX
    Core::AnimationData& data = Animations[animid];

    Core::aiSceneInfo& scene = Scenes[data.SceneInd];

// only aiSceneInfo ????

    float tickespersec = (float)(scene.TicksPerSecond != 0 ? scene.TicksPerSecond : 25.0f);

    float timeinticks = time * tickespersec;
    float timeticks =  fmod(timeinticks, (float)scene.Duration);
    
    //-------2------
    // scene.nodes.clear();
    // int iii = 0;
    // ReadNodes(scene.RootNode, scene.nodes, iii);
    // scene.rootssize = scene.nodes.size();
    // scene.timeticks = timeticks;
    // for (int i = 0; i < scene.rootssize; i++) {
    //     Gfx::NodeRootsCompute root;
    //     root.Index = scene.nodes[i].ind;
    //     root.Transform = scene.nodes[i].Transform;
    //     scene.noderoots[i] = root;
    // }
    // glm::mat4 globaltransf[200];
    // for (int i = 0; i < scene.rootssize; i++) {
    //     ReadNodeHierarchy3(i, model, scene, scene.nodes[i], scene.noderoots[i]);
    // }
    //
    // for (int i = 0; i < scene.rootssize; i++) {
    //     Hierarchy(model, scene, i, timeticks, glm::mat4(1), globaltransf);
    // }
    //-------1------
    // std::vector<glm::mat4> globaltransf(scene.nodes.size());
    // for (int j = 0; j < scene.nodes.size() ; j++) {
    //     ReadNodeHierarchy2(model, animid, scene,(scene.nodes[j]), timeticks, globaltransf, glm::mat4(1), j);
    // }
    //------0-----
    ReadNodeHierarchy(model, animid, scene, scene.RootNode, timeticks, glm::mat4(1.0));
#endif
}

glm::mat4 Core::AnimatorBase::InterpolatePos(glm::vec3 out, float timeticks, const NodeAnim& animnode)
{
    if (animnode.NumPositionsKeys == 1) {
        out = (animnode.PositionKeys[0]);
        return glm::translate(glm::mat4(1.0f), out);
    }

    u_int PositionIndex = FindPos(timeticks, animnode);
    u_int NextPositionIndex = PositionIndex + 1;
    // assert(NextPositionIndex < animnode.NumPositionsKeys);
    if (PositionIndex > animnode.PositionTime.size()) {
out = (animnode.PositionKeys[0]);
        return glm::translate(glm::mat4(1.0f), out);

    }
    float t1 = (float)animnode.PositionTime[PositionIndex];
    float t2 = (float)animnode.PositionTime[NextPositionIndex];
    float DeltaTime = t2 - t1;
    float Factor = (timeticks - t1) / DeltaTime;
    const glm::vec3& Start = animnode.PositionKeys[PositionIndex];
    const glm::vec3& End = animnode.PositionKeys[NextPositionIndex];
    glm::vec3 Delta = End - Start;
    out = glm::mix(Start, End, Factor);
    return  glm::translate(glm::mat4(1.0f), out);
// glm::mat4(1.0f);// glm::translate(glm::mat4(1.0f), out);
}

void Core::AnimatorBase::InterpolatePos(float timeticks, const NodeAnim& animnode, Core::aiSceneInfo& scene, int ind)
{
    glm::vec3 out;
    if (animnode.NumPositionsKeys == 1) {
        assert(animnode.NumPositionsKeys == 1);
        // scene.matricies.pos_out[ind] = glm::vec4(animnode.PositionKeys[0], 1.0);
        // scene.floats.pos_comp[ind] = 0; 
        return;
        // return glm::translate(glm::mat4(1.0f), out);
    }

    u_int PositionIndex = FindPos(timeticks, animnode);
    u_int NextPositionIndex = PositionIndex + 1;
    assert(NextPositionIndex < animnode.NumPositionsKeys);
    float t1 = (float)animnode.PositionTime[PositionIndex];
    float t2 = (float)animnode.PositionTime[NextPositionIndex];
    float DeltaTime = t2 - t1;
    scene.floats.pos_factor[ind] = (timeticks - t1) / DeltaTime;
    scene.matricies.pos_start[ind] = glm::vec4(animnode.PositionKeys[PositionIndex],1.0);
    scene.matricies.pos_end[ind] = glm::vec4( animnode.PositionKeys[NextPositionIndex], 1.0);
    // scene.floats.pos_comp[ind] = 1; 
    // const glm::vec3& End = animnode.PositionKeys[NextPositionIndex];
    // const glm::vec3& Start = animnode.PositionKeys[PositionIndex];
    // const glm::vec3& End = animnode.PositionKeys[NextPositionIndex];
    // glm::vec3 Delta = End - Start;
   // out = glm::mix(Start, End, Factor);
    // return  glm::translate(glm::mat4(1.0f), out);
// glm::mat4(1.0f);// glm::translate(glm::mat4(1.0f), out);
}
u_int Core::AnimatorBase::FindPos(float timeticks, const NodeAnim& animnode)
{
    assert(animnode.NumPositionsKeys > 0);
    
    int i = 0;
    for (float f : animnode.PositionTime) {
        if (timeticks < f) return i - 1;
        i++;
    }
    // for (u_int i = 0 ; i < animnode.NumPositionsKeys - 1 ; i++) {
    //     float t = (float)animnode.PositionTime[i + 1];
    //     if (timeticks < t) {
    //         return i;
    //     }
    // }
    return 0;
}

glm::mat4 Core::AnimatorBase::InterpolateRot(glm::quat out, float timeticks, const NodeAnim& animnode)
{
    if (animnode.NumRotationKeys == 1) {
        return glm::toMat4(static_cast<glm::quat>((animnode.RotationKeys[0])));
    }

    u_int RotationIndex = FindRot(timeticks, animnode);
    u_int NextRotationIndex = RotationIndex + 1;
    // assert(NextRotationIndex < animnode.NumRotationKeys);
    if (RotationIndex > animnode.RotationTime.size()) {

        return glm::toMat4(static_cast<glm::quat>((animnode.RotationKeys[0])));
    }
    float t1 = (float)animnode.RotationTime[RotationIndex];
    float t2 = (float)animnode.RotationTime[NextRotationIndex];
    float DeltaTime = t2 - t1;
    float Factor = (timeticks - t1) / DeltaTime;
    const glm::quat& StartRotationQ = animnode.RotationKeys[RotationIndex];
    const glm::quat& EndRotationQ = animnode.RotationKeys[NextRotationIndex];
    glm::quat finrot = glm::slerp(static_cast<glm::quat>(StartRotationQ), static_cast<glm::quat>(EndRotationQ), Factor);
    finrot = glm::normalize(finrot);
    return glm::toMat4(finrot);
}
void Core::AnimatorBase::InterpolateRot(float timeticks, const NodeAnim& animnode, aiSceneInfo& scene, int ind)
{
    if (animnode.NumRotationKeys == 1) {
        assert(animnode.NumRotationKeys == 1);
        // scene.matricies.rot_out[ind] = glm::toMat4(static_cast<glm::quat>((animnode.RotationKeys[0])));
        // scene.floats.rot_comp[ind] = 0; 
        return;
    }

    u_int RotationIndex = FindRot(timeticks, animnode);
    u_int NextRotationIndex = RotationIndex + 1;
    assert(NextRotationIndex < animnode.NumRotationKeys);
    float t1 = (float)animnode.RotationTime[RotationIndex];
    float t2 = (float)animnode.RotationTime[NextRotationIndex];
    float DeltaTime = t2 - t1;
    scene.floats.rot_factor[ind] = (timeticks - t1) / DeltaTime;
    scene.matricies.rot_start[ind] = glm::toMat4(animnode.RotationKeys[RotationIndex]);
    scene.matricies.rot_end[ind] = glm::toMat4(animnode.RotationKeys[NextRotationIndex]);
        // scene.floats.rot_comp[ind] = 1; 
    // glm::quat finrot = glm::slerp(static_cast<glm::quat>(StartRotationQ), static_cast<glm::quat>(EndRotationQ), Factor);
    // finrot = glm::normalize(finrot);
    // return glm::toMat4(finrot);
}

u_int Core::AnimatorBase::FindRot(float timeticks, const NodeAnim& animnode)
{
    assert(animnode.NumRotationKeys > 0);

    // for (u_int i = 0 ; i < animnode.NumRotationKeys - 1 ; i++) {
    //     float t = (float)animnode.RotationTime[i + 1];
    //     if (timeticks < t) {
    //         return i;
    //     }
    // }
    int i = 0;
    for (float f : animnode.RotationTime) {
        if (timeticks < f) return i - 1;
        i++;
    }
    return 0;
}

void Core::AnimatorBase::InterpolateScale(float timeticks, const NodeAnim& animnode, Core::aiSceneInfo& scene, int ind)
{
    if (animnode.NumScalingKeys == 1) {
        assert(animnode.NumScalingKeys == 1);
        // scene.matricies.scale_out[ind] = glm::vec4(animnode.ScalingKeys[0], 1.0);
        // scene.floats.scale_comp[ind] = 0; 
        return;

        // out = animnode.ScalingKeys[0];
        // return glm::scale(glm::mat4(1.0f), out);
    }

    u_int ScalingIndex = FindScale(timeticks, animnode);
    u_int NextScalingIndex = ScalingIndex + 1;
    assert(NextScalingIndex < animnode.NumScalingKeys);
    float t1 = (float)animnode.ScalingTime[ScalingIndex];
    float t2 = (float)animnode.ScalingTime[NextScalingIndex];
    float DeltaTime = t2 - t1;
    scene.floats.scale_factor[ind] = (timeticks - (float)t1) / DeltaTime;
    scene.matricies.scale_start[ind] = glm::vec4(animnode.ScalingKeys[ScalingIndex], 1.0);
    scene.matricies.scale_end[ind] = glm::vec4(animnode.ScalingKeys[NextScalingIndex], 1.0);
        // scene.floats.scale_comp[ind] = 1; 
    // glm::vec3 Delta = End - Start;
   // out = glm::mix(Start, End, Factor);//Start + Factor * Delta;
    // return glm::scale(glm::mat4(1.0f), out);
// glm::mat4(1.0f);// glm::scale(glm::mat4(1.0f), out);
}
glm::mat4  Core::AnimatorBase::InterpolateScale(glm::vec3 out, float timeticks, const NodeAnim& animnode)
{
    if (animnode.NumScalingKeys == 1) {
        out = animnode.ScalingKeys[0];
        return glm::scale(glm::mat4(1.0f), out);
    }

    u_int ScalingIndex = FindScale(timeticks, animnode);
    u_int NextScalingIndex = ScalingIndex + 1;
    //assert(NextScalingIndex < animnode.NumScalingKeys);
    if (ScalingIndex > animnode.ScalingTime.size()) {
out = animnode.ScalingKeys[0];
        return glm::scale(glm::mat4(1.0f), out);

    }
    float t1 = (float)animnode.ScalingTime[ScalingIndex];
    float t2 = (float)animnode.ScalingTime[NextScalingIndex];
    float DeltaTime = t2 - t1;
    float Factor = (timeticks - (float)t1) / DeltaTime;
    const glm::vec3& Start = animnode.ScalingKeys[ScalingIndex];
    const glm::vec3& End = animnode.ScalingKeys[NextScalingIndex];
    glm::vec3 Delta = End - Start;
    out = glm::mix(Start, End, Factor);//Start + Factor * Delta;
    return glm::scale(glm::mat4(1.0f), out);
}

u_int Core::AnimatorBase::FindScale(float timeticks, const NodeAnim& animnode)
{
     assert(animnode.NumScalingKeys > 0);

    // for (u_int i = 0 ; i < animnode.NumScalingKeys - 1 ; i++) {
    //     float t = (float)animnode.ScalingTime[i + 1];
    //     if (timeticks < t) {
    //         return i;
    //     }
    // }
int i = 0;
    for (float f : animnode.ScalingTime) {
        if (timeticks < f) return i - 1;
        i++;
    }

    return 0;
}

