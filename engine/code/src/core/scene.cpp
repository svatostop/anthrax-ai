#include "anthraxAI/core/scene.h"
#include "anthraxAI/core/audio.h"
#include "anthraxAI/core/imguihelper.h"
#include "anthraxAI/engine.h"
#include "anthraxAI/gamemodules/modules.h"
#include "anthraxAI/gameobjects/gameobjects.h"
#include "anthraxAI/gameobjects/objects/gizmo.h"
#include "anthraxAI/gameobjects/objects/light.h"
#include "anthraxAI/gameobjects/objects/sprite.h"
#include "anthraxAI/gfx/renderhelpers.h"
#include "anthraxAI/gfx/vkbase.h"
#include "anthraxAI/gfx/vkdescriptors.h"
#include "anthraxAI/gfx/vkrenderer.h"
#include "anthraxAI/gfx/model.h"
#include "anthraxAI/gfx/vkrendertarget.h"
#include "anthraxAI/utils/debug.h"
#include "anthraxAI/utils/mathdefines.h"
#include "anthraxAI/utils/parser.h"
#include "anthraxAI/utils/thread.h"
#include "anthraxAI/utils/tracy.h"
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <string>
#include <sys/types.h>
#include <utility>
#include <vector>
#include <vulkan/vulkan_core.h>

void Core::Scene::RenderThreaded(Modules::Module& module)
{
    if (!sec_cmds.empty()) {
        sec_cmds.clear();
    }
    else {
        sec_cmds.reserve(8);
    }
    VkFormat formats3[3] = { VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R16G16B16A16_SFLOAT };
    VkFormat depthformat = VK_FORMAT_D32_SFLOAT;

    uint32_t frameind = Gfx::Renderer::GetInstance()->GetFrameInd();

    u_int32_t obj_size = module.GetRenderQueue(Modules::RQ_GENERAL).size();
    uint32_t inst_ind = 0;//Gfx::Renderer::GetInstance()->GetInstanceInd();
    std::vector<uint32_t> num_obj_per_thread(Thread::MAX_RENDER_THREAD_NUM, (uint32_t)module.GetRenderQueue(Modules::RQ_GENERAL).size() / Thread::MAX_RENDER_THREAD_NUM );

    bool iseven = (module.GetRenderQueue(Modules::RQ_GENERAL).size() % Thread::MAX_RENDER_THREAD_NUM) == 0;
    if (!iseven) {
        num_obj_per_thread[num_obj_per_thread.size() - 1] += (module.GetRenderQueue(Modules::RQ_GENERAL).size() % Thread::MAX_RENDER_THREAD_NUM);
    }
    u_int32_t first_obj_size = 0;
    u_int32_t sec_obj_size = 0;// module.GetRenderQueue().size() / 2;

    uint32_t fin_inst_ind = 0;
    uint32_t fin_inst_ind2 = 0;
    std::vector<uint32_t> instance_inds(Thread::MAX_RENDER_THREAD_NUM + 1, 0);//= { 0, 0};
    for (uint32_t thread_id = 0; thread_id < Thread::MAX_RENDER_THREAD_NUM; thread_id++) {
        sec_obj_size += num_obj_per_thread[thread_id];
        for (uint32_t obj_num = first_obj_size; obj_num < sec_obj_size; obj_num++) {

            Gfx::RenderObject& obj = module.GetRenderQueue(Modules::RQ_GENERAL)[obj_num];
            instance_inds[thread_id + 1] += obj.Model[frameind]->Meshes.size();
        }
        first_obj_size = sec_obj_size;
    }
    first_obj_size = 0;
    sec_obj_size = 0;
    uint32_t inst = 0;
    for (uint32_t thread_id = 0; thread_id < Thread::MAX_RENDER_THREAD_NUM; thread_id++) {
        sec_obj_size += num_obj_per_thread[thread_id];

        inst += instance_inds[thread_id];
        //printf(" first obj %d === sec obj %d\n", first_obj_size, sec_obj_size);
        Thread::Pool::GetInstance()->PushByID(thread_id, { Thread::Task::Name::RENDER, Thread::Task::Type::EXECUTE,
        {}, [this, formats3, frameind, depthformat, thread_id,  &module, inst, first_obj_size, sec_obj_size]() {

        VkCommandBuffer secondary_cmd = Gfx::Renderer::GetInstance()->GetFrame().SecondaryCmd[thread_id].Cmd;
        VkCommandBufferBeginInfo secondary_cmd_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        VkCommandBufferInheritanceRenderingInfo inheritance_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO };
        inheritance_info.colorAttachmentCount = 3;
        inheritance_info.pColorAttachmentFormats = formats3;
        inheritance_info.depthAttachmentFormat = depthformat;
        inheritance_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        inheritance_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT;

        VkCommandBufferInheritanceInfo base_inheritance_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO };
        base_inheritance_info.pNext = &inheritance_info;
        secondary_cmd_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        secondary_cmd_info.pInheritanceInfo = &base_inheritance_info;

        vkBeginCommandBuffer(secondary_cmd, &secondary_cmd_info);

        uint32_t inst_ind = inst;
        for (uint32_t obj_num = first_obj_size; obj_num < sec_obj_size; obj_num++) {
            
            Gfx::RenderObject& obj = module.GetRenderQueue(Modules::RQ_GENERAL)[obj_num];

            if (!obj.IsVisible) {
                inst_ind += obj.Model[frameind]->Meshes.size();
                continue;
            }
  
            Gfx::MeshPushConstants constants;
            constants.texturebind = obj.TextureBind[frameind];
            constants.bufferbind = obj.BufferBind[frameind];
            if (obj.HasStorage) {
                constants.storagebind = obj.StorageBind[frameind];
                constants.instancebind = obj.InstanceBind[frameind];
            }

            const int meshsize = obj.Model[frameind]->Meshes.size();
            for (int i = 0; i < meshsize; i++) {
                Gfx::Renderer::GetInstance()->DrawThreaded(secondary_cmd, obj, obj.Material, obj.Model[frameind]->Meshes[i], constants, true, inst_ind);
                inst_ind++;
            }
        }
        vkEndCommandBuffer(secondary_cmd);
        },{},  nullptr, 0,  nullptr, nullptr, nullptr});

        first_obj_size = sec_obj_size;
    }

   if (module.GetTag() == "gbuffer") {
        Thread::Pool::GetInstance()->WaitRender();
        Gfx::Renderer::GetInstance()->SetInstanceInd(fin_inst_ind2);
        for (int cb = 0; cb < Thread::MAX_RENDER_THREAD_NUM; cb++) {
            sec_cmds.emplace_back(Gfx::Renderer::GetInstance()->GetFrame().SecondaryCmd[cb].Cmd);
        }
        Gfx::Renderer::GetInstance()->SetCmd(Gfx::Renderer::GetInstance()->GetFrame().MainCommandBuffer);
        if (!sec_cmds.empty()) {
            vkCmdExecuteCommands(Gfx::Renderer::GetInstance()->GetCmd(),sec_cmds.size(), sec_cmds.data());
    }
    }
}

void Core::Scene::Compute(Modules::Module& module, uint32_t work_groups, uint32_t work_groups2 )
{
    Gfx::Renderer::GetInstance()->DebugRenderName(module.GetTag());
 
    uint32_t frameind = Gfx::Renderer::GetInstance()->GetFrameInd();
    for (auto& it : module.GetRenderQueueMap()) {
        for (Gfx::RenderObject& obj : it.second) {
            if (module.GetTag() == "particles") {
                Gfx::Renderer::GetInstance()->ComputeParticles(obj);
            }
            else {
                Gfx::Renderer::GetInstance()->Compute(obj, work_groups, work_groups2);
            }
        }
    }
    
    Gfx::Renderer::GetInstance()->EndRenderName();
}

void Core::Scene::Render(Modules::Module& module)
{
    Gfx::Renderer::GetInstance()->DebugRenderName(module.GetTag());

    if (( module.GetTag() == "gbuffer" && module.GetRenderQueue(Modules::RQ_GENERAL).size() > Thread::MAX_RENDER_THREAD_NUM * 2) ){
        RenderThreaded(module);
    }
    else {
        uint32_t frameind = Gfx::Renderer::GetInstance()->GetFrameInd();
        for (auto& it : module.GetRenderQueueMap()) {
        for (Gfx::RenderObject& obj : it.second) {
            if (!obj.IsVisible) {
                Gfx::Renderer::GetInstance()->IncInstanceInd(obj.Model[frameind]->Meshes.size());
                continue;
            }
            if (module.GetTag() == "mask" && !obj.IsSelected) {
                Gfx::Renderer::GetInstance()->IncInstanceInd(obj.Model[frameind]->Meshes.size());
                continue;
            }
            if (obj.VertexBase || obj.IsCompute) {
                Gfx::Renderer::GetInstance()->DrawSimple(obj);
            }
            else {
                Gfx::Renderer::GetInstance()->Draw(obj);
            }
        }
        }
    }
    Gfx::Renderer::GetInstance()->EndRenderName();
}

void Core::Scene::RenderScene(bool playmode)
{
#ifdef TRACY
    //START_FRAME("frame") 
    ZoneScopedN("Scene::RenderScene");
#endif
    if (Gfx::Renderer::GetInstance()->BeginFrame()) {
        RenderPassed = true;
        Thread::Pool::GetInstance()->Time.BeginTime(Thread::Task::Name::RENDER, (double)Gfx::Renderer::GetInstance()->Time);
        
        Gfx::Renderer::GetInstance()->PrepareInstanceBuffer();
        Gfx::Renderer::GetInstance()->PrepareCameraBuffer(*EditorCamera);
        {
#ifdef TRACY
        TracyVkZoneC(Gfx::Renderer::GetInstance()->GetTracyContext(), Gfx::Renderer::GetInstance()->GetCmd(), "VulkanBeginRenderingModules", tracy::Color::Red);
#endif
            // used for intro
            if (!HasGBuffer) {
                Gfx::Renderer::GetInstance()->StartRender(GameModules->Get(CurrentScene).GetIAttachments(), Gfx::AttachmentRules::ATTACHMENT_RULE_CLEAR);
                Render(GameModules->Get(CurrentScene));
                Gfx::Renderer::GetInstance()->EndRender();
            }
            else {
#ifdef COMPUTE_MTX
                if (playmode) {
                    Gfx::Renderer::GetInstance()->GetCmdHandle().MemoryBarrier(
                            Gfx::DescriptorsBase::GetInstance()->GetBuffer(Gfx::Renderer::GetInstance()->GetFrameInd(), Gfx::INSTANCE),
                            sizeof(Gfx::InstanceData) * MAX_INSTANCES,
                            VK_ACCESS_SHADER_READ_BIT,
                            VK_ACCESS_SHADER_WRITE_BIT,
                            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
                            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
                    );
                    Compute(GameModules->Get("compute_mtx"), u_int32_t(GameModules->GetAnimationIndexSize()) );
                    Gfx::Renderer::GetInstance()->GetCmdHandle().MemoryBarrier(
                            Gfx::DescriptorsBase::GetInstance()->GetBuffer(Gfx::Renderer::GetInstance()->GetFrameInd(), Gfx::INSTANCE),
                            sizeof(Gfx::InstanceData) * MAX_INSTANCES,
                            VK_ACCESS_SHADER_WRITE_BIT,
                            VK_ACCESS_SHADER_READ_BIT,
                            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
                    );
                }
#endif
#ifdef COMPUTE_SKINNING
                {
                    Gfx::Renderer::GetInstance()->GetCmdHandle().MemoryBarrier(
                            Gfx::DescriptorsBase::GetInstance()->GetBuffer(0, Gfx::SKINNING_OUT),
                            sizeof(Gfx::VertexOutputData) * GetSceneVertSize(),
                            VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
                            VK_ACCESS_SHADER_WRITE_BIT,
                            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
                    );
                    Gfx::Renderer::GetInstance()->GetCmdHandle().MemoryBarrier(
                            Gfx::DescriptorsBase::GetInstance()->GetBuffer(0, Gfx::SKINNING_IN),
                            sizeof(Gfx::VertexInputData) * Gfx::Model::GetInstance()->GetVerteciesSize(),
                            VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
                            VK_ACCESS_SHADER_WRITE_BIT,
                            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
                    );

                    // Gfx::Renderer::GetInstance()->BarrierVertexCompute(0);
                    Compute(GameModules->Get("compute_skinning"), ((Gfx::Renderer::GetInstance()->ComputeSkinningMaxVertex )+ 255) / 256);// (Gfx::Renderer::GetInstance()->ComputeSkinningMaxVertex + 32));
                    // Gfx::Renderer::GetInstance()->BarrierVertexCompute(1);
                    Gfx::Renderer::GetInstance()->GetCmdHandle().MemoryBarrier(
                            Gfx::DescriptorsBase::GetInstance()->GetBuffer(0, Gfx::SKINNING_OUT),
                            sizeof(Gfx::VertexOutputData) * GetSceneVertSize(),
                            VK_ACCESS_SHADER_WRITE_BIT,
                            VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
                            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT
                    );
                    Gfx::Renderer::GetInstance()->GetCmdHandle().MemoryBarrier(
                            Gfx::DescriptorsBase::GetInstance()->GetBuffer(0, Gfx::SKINNING_IN),
                            sizeof(Gfx::VertexInputData) * Gfx::Model::GetInstance()->GetVerteciesSize(),
                            VK_ACCESS_SHADER_WRITE_BIT,
                            VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
                            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT
                    );

                    //
                    Gfx::Renderer::GetInstance()->GetCmdHandle().MemoryBarrier(
                            Gfx::DescriptorsBase::GetInstance()->GetBuffer(Gfx::Renderer::GetInstance()->GetFrameInd(), Gfx::INSTANCE),
                            sizeof(Gfx::InstanceData) * MAX_INSTANCES,
                            VK_ACCESS_SHADER_READ_BIT,
                            VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
                            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT
                    );

                }
#endif
                if (HasCompute) {
                    Compute(GameModules->Get("particles"), 0);
                    
                    Gfx::Renderer::GetInstance()->StartRender(GameModules->Get("particles").GetIAttachments(), Gfx::AttachmentRules::ATTACHMENT_RULE_CLEAR);
                    Render(GameModules->Get("particles"));
                    Gfx::Renderer::GetInstance()->EndRender();
                }
                else {
// #ifndef COMPUTE_SKINNING
                    if (Gfx::Renderer::GetInstance()->GetCubemapRendering()) {
                        Gfx::Renderer::GetInstance()->StartRender(GameModules->Get("lighting").GetIAttachments(), Gfx::AttachmentRules::ATTACHMENT_RULE_CLEAR);
                        Render(GameModules->Get("skybox"));
                        Gfx::Renderer::GetInstance()->EndRender();
                    }
            // #endif
                }
                // objects from map
#ifdef DRAW_INDIRECT
#ifdef VISIBILITY_COMPUTE
                {
                    // Compute(GameModules->Get("visibility_compute"), 64);
                }
#else
                Gfx::Renderer::GetInstance()->ResetSkinningIterator();
                Gfx::Renderer::GetInstance()->ClearIndirectBatches();
                Gfx::Renderer::GetInstance()->CompactDrawIndirect(GameModules->Get("gbuffer").GetRenderQueueMap());
    Gfx::Renderer::GetInstance()->DebugRenderName(GameModules->Get("gbuffer").GetTag());
                Gfx::Renderer::GetInstance()->StartRender(GameModules->Get("gbuffer").GetIAttachments(), Gfx::AttachmentRules::ATTACHMENT_RULE_CLEAR, false);
                // Gfx::Renderer::GetInstance()->RenderIndirect(GameModules->Get("gbuffer").GetRenderQueueMap());
                Gfx::Renderer::GetInstance()->RenderIndirect(GameModules->Get("gbuffer"));
                Gfx::Renderer::GetInstance()->EndRender();
    Gfx::Renderer::GetInstance()->EndRenderName();
#endif
                if (HasFrameShadows) { 
                    Gfx::Renderer::GetInstance()->ResetSkinningIterator();
                    Gfx::Renderer::GetInstance()->ClearIndirectBatches();
                    Gfx::Renderer::GetInstance()->CompactDrawIndirect(GameModules->Get("shadows").GetRenderQueueMap());
    Gfx::Renderer::GetInstance()->DebugRenderName(GameModules->Get("shadows").GetTag());
                    Gfx::Renderer::GetInstance()->StartRender(GameModules->Get("shadows").GetIAttachments(), Gfx::AttachmentRules::ATTACHMENT_RULE_CLEAR);
                    // Gfx::Renderer::GetInstance()->RenderIndirect(GameModules->Get("shadows").GetRenderQueueMap());
                    Gfx::Renderer::GetInstance()->RenderIndirect(GameModules->Get("shadows"));
                    Gfx::Renderer::GetInstance()->EndRender();
    Gfx::Renderer::GetInstance()->EndRenderName();

                }

                
#else
                Gfx::Renderer::GetInstance()->ResetSkinningIterator();
                Gfx::Renderer::GetInstance()->StartRender(GameModules->Get("gbuffer").GetIAttachments(), Gfx::AttachmentRules::ATTACHMENT_RULE_CLEAR, GameModules->Get("gbuffer").GetRenderQueue(Modules::RQ_GENERAL).size() > Thread::MAX_RENDER_THREAD_NUM * 2 ? true : false);
                Render(GameModules->Get("gbuffer"));
                Gfx::Renderer::GetInstance()->EndRender();
                if (HasFrameShadows) { 
                    Gfx::Renderer::GetInstance()->ResetSkinningIterator();
                    Gfx::Renderer::GetInstance()->ResetInstanceInd();
                    Gfx::Renderer::GetInstance()->StartRender(GameModules->Get("shadows").GetIAttachments(), Gfx::AttachmentRules::ATTACHMENT_RULE_CLEAR);
                    Render(GameModules->Get("shadows"));
                    Gfx::Renderer::GetInstance()->EndRender();
                }
#endif
                Gfx::AttachmentRules rule = Gfx::Renderer::GetInstance()->GetCubemapRendering() || HasCompute ? Gfx::AttachmentRules::ATTACHMENT_RULE_LOAD : Gfx::AttachmentRules::ATTACHMENT_RULE_CLEAR;
                Gfx::Renderer::GetInstance()->StartRender(GameModules->Get("lighting").GetIAttachments(), rule);
                Render(GameModules->Get("lighting"));
                Gfx::Renderer::GetInstance()->EndRender();

            }

            if (HasFrameGrid && Utils::Debug::GetInstance()->Grid) {

                Gfx::Renderer::GetInstance()->StartRender(GameModules->Get("grid").GetIAttachments(), Gfx::AttachmentRules::ATTACHMENT_RULE_LOAD);
                Render(GameModules->Get("grid"));
                Gfx::Renderer::GetInstance()->EndRender();
            }
            if (GameModules->Find("sprite")) {
                Gfx::Renderer::GetInstance()->StartRender(GameModules->Get("sprite").GetIAttachments(), Gfx::AttachmentRules::ATTACHMENT_RULE_LOAD);
                Render(GameModules->Get("sprite"));
                Gfx::Renderer::GetInstance()->EndRender();
            }
            #ifndef COMPUTE_SKINNING
            Gfx::Renderer::GetInstance()->CopyImage(Gfx::RT_MAIN_COLOR, Gfx::RT_MAIN_DEBUG);
            #endif
            // gizmo, outline
            if (playmode && HasFrameGizmo) {
                
                if (GameModules->HasFrameOutline()) {
                    Gfx::Renderer::GetInstance()->ResetInstanceInd();
                    Gfx::Renderer::GetInstance()->GetRT(Gfx::RT_MASK)->MemoryBarrier(Gfx::Renderer::GetInstance()->GetCmd(),VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
                    Gfx::Renderer::GetInstance()->StartRender(GameModules->Get("mask").GetIAttachments(), Gfx::AttachmentRules::ATTACHMENT_RULE_CLEAR);
                    Render(GameModules->Get("mask"));
                    Gfx::Renderer::GetInstance()->EndRender();

                    Gfx::Renderer::GetInstance()->StartRender(GameModules->Get(CurrentScene).GetIAttachments(), Gfx::AttachmentRules::ATTACHMENT_RULE_LOAD);
                    Render(GameModules->Get("outline"));
                    Gfx::Renderer::GetInstance()->EndRender();
                }
                if (HasFrameGizmo) {
                    Gfx::Renderer::GetInstance()->StartRender(GameModules->Get("gizmo").GetIAttachments(), Gfx::AttachmentRules::ATTACHMENT_RULE_LOAD);
                    Render(GameModules->Get("gizmo"));
                    Gfx::Renderer::GetInstance()->EndRender();
                }
            }
            // ui
            if (HasGBuffer) {
                Gfx::Renderer::GetInstance()->TransferLayoutsDebug();
            }
            Gfx::Renderer::GetInstance()->StartRender(GameModules->Get(CurrentScene).GetIAttachments(), static_cast<Gfx::AttachmentRules>(Gfx::AttachmentRules::ATTACHMENT_RULE_LOAD));
            Gfx::Renderer::GetInstance()->RenderUI();
            Gfx::Renderer::GetInstance()->EndRender();
        }

        Thread::Pool::GetInstance()->Time.EndTime(Thread::Task::Name::RENDER, (double)Engine::GetInstance()->GetTime());
        if (GameModules->Get(CurrentScene).GetStorageBuffer()) {
              Gfx::Renderer::GetInstance()->PrepareStorageBuffer();
        }

       // Thread::PrintTime(Thread::Task::Name::RENDER);
        
        Gfx::Renderer::GetInstance()->EndFrame();
    }
    else {
        RenderPassed = false;
    }
#ifdef TRACY
    //END_FRAME("frame"); 
#endif
}

void Core::Scene::Loop()
{
#ifdef TRACY
    ZoneScopedN("Scene::Loop");
#endif
    Core::Audio::GetInstance()->Play();
    
    if (Utils::IsBitSet(Engine::GetInstance()->GetState(), ENGINE_STATE_RESOURCE_RELOAD)) {
        ReloadResources();
    }
    if (Utils::IsBitSet(Engine::GetInstance()->GetState(), ENGINE_STATE_SHADER_RELOAD)) {
        GameModules->Update(Modules::Update::MATERIALS);
        Engine::GetInstance()->ClearState(ENGINE_STATE_SHADER_RELOAD);
    }
    GameModules->Update(Modules::Update::SAMPLERS);

    if (Utils::IsBitSet(Engine::GetInstance()->GetState(), ENGINE_STATE_INTRO)) {
        RenderScene(false);
    }
    if (Utils::IsBitSet(Engine::GetInstance()->GetState(), ENGINE_STATE_EDITOR)) {
        Core::ImGuiHelper::GetInstance()->Render();

        Thread::Pool::GetInstance()->Pause(true);
        // if (Thread::Pool::GetInstance()->IsInit()) {
        //     Thread::Pool::GetInstance()->WaitWork();
        // }
        
        RenderScene(false);
    }
    if (Utils::IsBitSet(Engine::GetInstance()->GetState(), ENGINE_STATE_PLAY)) {
        if (HasEditor) {
            Core::ImGuiHelper::GetInstance()->Render();
        }
        Thread::Pool::GetInstance()->Pause(false);
        Thread::Pool::GetInstance()->Time.BeginTime(Thread::Task::Name::UPDATE, (double)Engine::GetInstance()->GetTime());

        GameObjects->Update();
        GameModules->Update(Modules::Update::RQ);

        Thread::Pool::GetInstance()->Time.EndTime(Thread::Task::Name::UPDATE, (double)Engine::GetInstance()->GetTime());

        RenderScene(true);
    }
    GameModules->Update(Modules::Update::TEXTURE_UI_MANAGER);
}

void Core::Scene::Init()
{
    ParseSceneNames();

    GameObjects = new Keeper::Base;
    GameObjects->Create<Keeper::Camera>(new Keeper::Camera(Keeper::Camera::Type::EDITOR, {10.0f, 10.0f, 3.0f}));
    EditorCamera = reinterpret_cast<Keeper::Camera*>(*(GameObjects->Get(Keeper::Type::CAMERA).begin()));
}

void Core::Scene::InitModules()
{
    Thread::Pool::GetInstance()->Init(4);

    GameModules = new Modules::Base(GameObjects);

    Modules::Info info;
    info.BindlessType = Gfx::BINDLESS_DATA_CAM_BUFFER ;
    info.IAttachments.Add(Gfx::RT_MAIN_COLOR);
    GameModules->Populate("intro", info,
        GameObjects->GetInfo(Keeper::Infos::INFO_INTRO)
    );
    GameModules->Update(Modules::Update::RESOURCES);
    Core::Audio::GetInstance()->Load("Anthrax_Mastered.wav");
    Core::Audio::GetInstance()->SetVolume(0.0f);
}

void Core::Scene::NewScene()
{
    Gfx::Renderer::GetInstance()->SetCubemapRendering(false);
    HasFrameShadows = false;
    SetCurrentScene("New Scene");
    CurrentSceneForUpdate = "New Scene"; 
    ParsedSceneInfo.clear();
    
    IsNewScene = true;

    Keeper::Info info;
    info.ParsedID = "cube";
    info.Position = Vector3<float>(0.0f,0.0f, 0.0f);
    info.Material = "models";
    info.Vertex = "model.vert";
    info.Fragment = "model.frag";
    info.Texture = "dummy.png";
    info.Model = "cube.obj";
    info.IsModel = true;
    ParsedSceneInfo.push_back(info);
}

void Core::Scene::ReloadResources()
{
    Thread::Pool::GetInstance()->Reload();

    if (!IsNewScene) {
        CurrentSceneForUpdate = CurrentScene;
        ParsedSceneInfo.clear();
        ParsedSceneInfo.reserve(10);
        LoadScene(CurrentScene);
    }
    IsNewScene = false;
    GameObjects->CleanIfNot(Keeper::Type::CAMERA, true);

    GameObjects->Create(ParsedSceneInfo);
    GameObjects->Create<Keeper::Gizmo>(new Keeper::Gizmo(GameObjects->GetGizmoInfo(Keeper::Gizmo::Type::Y), Keeper::Gizmo::Type::Y));
    GameObjects->Create<Keeper::Gizmo>(new Keeper::Gizmo(GameObjects->GetGizmoInfo(Keeper::Gizmo::Type::X), Keeper::Gizmo::Type::X));
    GameObjects->Create<Keeper::Gizmo>(new Keeper::Gizmo(GameObjects->GetGizmoInfo(Keeper::Gizmo::Type::Z), Keeper::Gizmo::Type::Z));
    EditorCamera->SetPosition({5.0f, 10.0f, -3.0f});

    Gfx::Vulkan::GetInstance()->ReloadResources();
    Core::Audio::GetInstance()->ResetState();

    Engine::GetInstance()->ClearState(ENGINE_STATE_RESOURCE_RELOAD);
    Engine::GetInstance()->SetState(ENGINE_STATE_EDITOR);

    PopulateModules();

    GameObjects->UpdateObjectNames();
    Core::ImGuiHelper::GetInstance()->UpdateObjectInfo();
}

void Core::Scene::ParseSceneNames()
{
    std::string path = "scenes/";

    SceneNames.reserve(20);
    for (const auto& name : std::filesystem::directory_iterator(path)) {
        std::string str = name.path().string();
        std::string basename = str.substr(str.find_last_of("/\\") + 1);
        SceneNames.push_back(basename.c_str());
    }
}

void Core::Scene::PopulateModules()
{
    GameModules->Clear();
    GameModules->SetCurrentScene(CurrentScene);

    {
        Modules::Info info;
        info.BindlessType = Gfx::BINDLESS_DATA_CAM_STORAGE_SAMPLER ;
        info.IAttachments.Add(Gfx::RT_MAIN_COLOR);
        info.IAttachments.Add(Gfx::RT_DEPTH, true);
        GameModules->Populate(CurrentScene, info,
            [](Keeper::Type t) { return t == Keeper::CAMERA || t == Keeper::GIZMO || t == Keeper::SPRITE; }
        );
    }

    bool sprite = GameObjects->Find(Keeper::SPRITE);
    bool npc = GameObjects->Find(Keeper::NPC);
        
    if (sprite) {
        Modules::Info info;
        info.BindlessType = Gfx::BINDLESS_DATA_CAM_STORAGE_SAMPLER ;
        info.IAttachments.Add(Gfx::RT_MAIN_COLOR);
        GameModules->Populate("sprite", info, GameObjects->GetObjects(Keeper::SPRITE));
    }
    HasFrameGizmo = false;
    HasFrameGrid = false;
    HasGBuffer = false;
    HasCompute = false;
    if (npc) {
        {
            Modules::Info info;
            info.BindlessType = Gfx::BINDLESS_DATA_CAM_STORAGE_SAMPLER ;
            info.IAttachments.AddA(Gfx::RT_ALBEDO);
            info.IAttachments.AddP(Gfx::RT_POSITION);
            info.IAttachments.AddN(Gfx::RT_NORMAL);
            info.IAttachments.Add(Gfx::RT_DEPTH, true);
            GameModules->Populate("gbuffer", info,
                GameObjects->GetInfo(Keeper::Infos::INFO_GBUFFER)
            );
            HasGBuffer = true;
        }
        {
            Modules::Info info;
            info.BindlessType = Gfx::BINDLESS_DATA_CAM_STORAGE_SAMPLER ;
            info.IAttachments.Add(Gfx::RT_SHADOWS, true);
            GameModules->Populate("shadows", info,
                GameObjects->GetInfo(Keeper::Infos::INFO_SHADOWS)
            );
        }
        {
            Modules::Info info;
            info.BindlessType = Gfx::BINDLESS_DATA_CAM_STORAGE_SAMPLER ;
            info.IAttachments.Add(Gfx::RT_MAIN_COLOR);
            GameModules->Populate("skybox", info,
                GameObjects->GetInfo(Keeper::Infos::INFO_SKYBOX)
            );
        }
        {
            Modules::Info info;
            info.BindlessType = Gfx::BINDLESS_DATA_CAM_STORAGE_SAMPLER ;
            info.IAttachments.Add(Gfx::RT_MAIN_COLOR);
            GameModules->Populate("lighting", info,
                GameObjects->GetInfo(Keeper::Infos::INFO_LIGHTING)
            );
        }
        {
            Modules::Info info;
            info.BindlessType = Gfx::BINDLESS_DATA_CAM_STORAGE_SAMPLER ;
            info.IAttachments.Add(Gfx::RT_MAIN_COLOR);

            GameModules->Populate("gizmo", info,
                [](Keeper::Type t) { return t != Keeper::GIZMO; }
            );
        }
        {
            Modules::Info info;
            info.BindlessType = Gfx::BINDLESS_DATA_CAM_BUFFER ;
            info.IAttachments.Add(Gfx::RT_MAIN_COLOR);
            info.IAttachments.Add(Gfx::RT_DEPTH, true);

            GameModules->Populate("grid", info,
                GameObjects->GetInfo(Keeper::Infos::INFO_GRID)
            );
        }
        {
            Modules::Info info;
            info.BindlessType = Gfx::BINDLESS_DATA_CAM_STORAGE_SAMPLER ;
            info.IAttachments.Add(Gfx::RT_MASK);

            GameModules->Populate("mask", info,
                GameObjects->GetInfo(Keeper::Infos::INFO_MASK)
            );
        }
        {
            Modules::Info info;
            info.BindlessType = Gfx::BINDLESS_DATA_CAM_STORAGE_SAMPLER ;
            info.IAttachments.Add(Gfx::RT_MAIN_COLOR);
            GameModules->Populate("outline", info,
                GameObjects->GetInfo(Keeper::Infos::INFO_OUTLINE)
            );
        }
        {
            Modules::Info info;
            info.BindlessType = Gfx::BINDLESS_DATA_PARTICLES;
            info.IAttachments.Add(Gfx::RT_MAIN_COLOR);
            GameModules->Populate("particles", info,
                GameObjects->GetInfo(Keeper::Infos::INFO_PARTICLES)
            );
            Gfx::Renderer::GetInstance()->PrepareCompute();
        }
#ifdef COMPUTE_MTX
        {
            Modules::Info info;
            info.BindlessType = Gfx::BINDLESS_DATA_STORAGE;
            info.IAttachments.Add(Gfx::RT_MAIN_COLOR);
            GameModules->Populate("compute_mtx", info,
                GameObjects->GetInfo(Keeper::Infos::INFO_COMPUTE_MTX)
            );
        }
#endif 
#ifdef VISIBILITY_COMPUTE
        {
            Modules::Info info;
            info.BindlessType = Gfx::BINDLESS_DATA_STORAGE;
            info.IAttachments.Add(Gfx::RT_VISIBILITY);
            GameModules->Populate("visibility_compute", info,
                GameObjects->GetInfo(Keeper::Infos::INFO_VISIBILITY_COMPUTE)
            );

        }
#endif
#ifdef COMPUTE_SKINNING
        {
            Modules::Info info;
            info.BindlessType = Gfx::BINDLESS_DATA_SKINNING;
            info.IAttachments.Add(Gfx::RT_MAIN_COLOR);
            GameModules->Populate("compute_skinning", info,
                GameObjects->GetInfo(Keeper::Infos::INFO_COMPUTE_SKINNING)
            );

        }
        GameModules->RestartAnimator();
#ifdef COMPUTE_SKINNING
	Gfx::DescriptorsBase::GetInstance()->AllocateSkinningBuffer();
    Gfx::Renderer::GetInstance()->FillSkinningBuffer();
#endif

#endif
        HasFrameGrid = true;
        HasFrameGizmo = true;
    }
    GameModules->Update(Modules::Update::RESOURCES, true);
}

void Core::Scene::ClearNewObjectInfo()
{
   GameObjects->ClearNewObjectInfo(); 
}

void Core::Scene::DeleteSelectedObject()
{
    if (GetSelectedID() == -1 || !GameObjects->EraseSelected()) {
        return;
    }
    Thread::Pool::GetInstance()->Reload();
    GameModules->EraseSelected();
    GameObjects->UpdateObjectNames();
    Core::ImGuiHelper::GetInstance()->UpdateObjectInfo();
    
    if (GameModules->Get("gbuffer").GetRenderQueue(Modules::RQ_GENERAL).empty()) {
        HasFrameGizmo = false;
    }
}

void Core::Scene::SaveObject() 
{
    GameObjects->VerifyNewObject();
    Keeper::Type type;
    if (GameObjects->NewObjectInfo.Type == "Light") {
        GameObjects->Create<Keeper::Light>(new Keeper::Light(GameObjects->NewObjectInfo, GameObjects->NewObjectInfo.ParsedID));
        type = Keeper::Type::LIGHT;
    }
    else if (GameObjects->NewObjectInfo.Type == "Sprite") {
        GameObjects->Create<Keeper::Sprite>(new Keeper::Sprite(GameObjects->NewObjectInfo)); 
        type = Keeper::Type::SPRITE;
    }
    else {
        GameObjects->Create<Keeper::Npc>(new Keeper::Npc(GameObjects->NewObjectInfo)); 
        type = Keeper::Type::NPC;
    }
    GameModules->Insert(GameObjects->GetLast(type));
    GameObjects->UpdateObjectNames();
    Core::ImGuiHelper::GetInstance()->UpdateObjectInfo();

    if (!GameModules->Get("gbuffer").GetRenderQueue(Modules::RQ_GENERAL).empty()) {
        HasFrameGizmo = true;
    }

}        
void Core::Scene::SetCurrentScene(const std::string& str)
{
    CurrentScene = str;
    Engine::GetInstance()->ClearState(ENGINE_STATE_PLAY);
    Engine::GetInstance()->ClearState(ENGINE_STATE_EDITOR);
    Engine::GetInstance()->SetState(ENGINE_STATE_RESOURCE_RELOAD);

}

void Core::Scene::ExportScene()
{
    Parse.ClearFile(CurrentSceneForUpdate);
    Parse.Clear();

    Parse.AddRoot(Utils::LEVEL_ELEMENT_SCENE, CurrentSceneForUpdate);
    if (HasFrameShadows) {
        Parse.AddRoot(Utils::LEVEL_ELEMENT_SHADOWS, "");
    }
    if (Gfx::Renderer::GetInstance()->GetCubemapRendering()) {
        Parse.AddRoot(Utils::LEVEL_ELEMENT_CUBEMAPS, "");
        Parse.AddRoot(Utils::LEVEL_ELEMENT_NAME, GameModules->GetCubemapTexture());
    }
    Parse.Write(CurrentSceneForUpdate);
    
    for (auto& it : GameObjects->GetObjects()) {
        if (it.first != Keeper::NPC && it.first != Keeper::SPRITE && it.first != Keeper::LIGHT) continue;
        for (Keeper::Objects* obj : it.second) {
            ExportObjectInfo(obj);
        }
    }
}

void Core::Scene::ExportObjectInfo(const Keeper::Objects* obj)
{
    std::string rootname = Parse.GetRootElement();

    Utils::NodeIt obj_node = Parse.GetChildByID(Parse.GetRootNode(), obj->GetParsedID());

    if (!Parse.IsNodeValid(obj_node)) {
        Parse.UpdateTokens(obj); 
        Parse.Write(CurrentSceneForUpdate);
        return;
    }

    Utils::NodeIt obj_texture = Parse.GetChild(obj_node, Utils::LEVEL_ELEMENT_TEXTURE);
    Utils::NodeIt obj_textname = Parse.GetChild(obj_texture, Utils::LEVEL_ELEMENT_NAME);
    if (Parse.IsNodeValid(obj_textname)) {
        Parse.UpdateElement(obj_textname, obj->GetTextureName());
    }
    Utils::NodeIt obj_mat = Parse.GetChild(obj_node, Utils::LEVEL_ELEMENT_MATERIAL);
    Utils::NodeIt obj_matname = Parse.GetChild(obj_mat, Utils::LEVEL_ELEMENT_NAME);
    if (Parse.IsNodeValid(obj_matname)) {
        Parse.UpdateElement(obj_matname, obj->GetMaterialName());
    }
    
    Utils::NodeIt obj_mod = Parse.GetChild(obj_node, Utils::LEVEL_ELEMENT_MODEL);
    Utils::NodeIt obj_modname = Parse.GetChild(obj_mod, Utils::LEVEL_ELEMENT_NAME);
    if (Parse.IsNodeValid(obj_modname)) {
        Parse.UpdateElement(obj_modname, obj->GetModelName());
    }

    Utils::NodeIt obj_position = Parse.GetChild(obj_node, Utils::LEVEL_ELEMENT_POSITION);
    if (Parse.IsNodeValid(obj_position)) {
        Utils::NodeIt x = Parse.GetChild(obj_position, Utils::LEVEL_ELEMENT_X);
        Parse.UpdateElement(x, std::to_string(obj->GetPosition().x));
        Utils::NodeIt y = Parse.GetChild(obj_position, Utils::LEVEL_ELEMENT_Y);
        Parse.UpdateElement(y, std::to_string(obj->GetPosition().y));
        Utils::NodeIt z = Parse.GetChild(obj_position, Utils::LEVEL_ELEMENT_Z);
        Parse.UpdateElement(z, std::to_string(obj->GetPosition().z));
    }

    Parse.Write(CurrentSceneForUpdate);
}

void Core::Scene::LoadScene(const std::string& filename)
{
    Parse.Clear();
    Parse.Load(filename);

    std::string scenename = Parse.GetRootElement();

    float xpos, ypos, zpos = 0.0f;
    std::string matname, textname, modname, frag, vert, id;
     
    Gfx::Renderer::GetInstance()->SetCubemapRendering(false);
    HasFrameShadows = false;
    Utils::NodeIt cube_node = Parse.GetChild(Parse.GetRootNode(), Utils::LEVEL_ELEMENT_CUBEMAPS);
    if (Parse.IsNodeValid(cube_node)) {
        std::string texture = Parse.GetElement<std::string>(cube_node, Utils::LEVEL_ELEMENT_NAME, "");
        if (!texture.empty()) {
            Gfx::Renderer::GetInstance()->SetCubemapRendering(true);
            GameModules->SetCubemapTexture(texture);
        }
    }
    Utils::NodeIt shadows_node = Parse.GetChild(Parse.GetRootNode(), Utils::LEVEL_ELEMENT_SHADOWS);
    if (Parse.IsNodeValid(cube_node)) {
        HasFrameShadows = true;    
    }

    Utils::NodeIt node = Parse.GetChild(Parse.GetRootNode(), Utils::LEVEL_ELEMENT_OBJECT);
    while (Parse.IsNodeValid(node)) {
        Keeper::Info info;
        id = Parse.GetElement<std::string>(node, Utils::LEVEL_ELEMENT_ID, "");
        info.ParsedID = id;

        // Utils::NodeIt rot = Parse.GetChild(node, Utils::LEVEL_ELEMENT_ROTATION);
        // if (Parse.IsNodeValidInRange(rot)) {
        //     xpos = Parse.GetElement<float>(rot, Utils::LEVEL_ELEMENT_X, 0.0);
        //     ypos = Parse.GetElement<float>(rot, Utils::LEVEL_ELEMENT_Y, 0.0);
        //     zpos = Parse.GetElement<float>(rot, Utils::LEVEL_ELEMENT_Z, 0.0);
        //     info.Rotation = Vector3<float>( xpos, ypos, zpos );
        // }
        //
        // info.Scale = 1;// Vector3<float>( xpos, ypos, zpos );
        // Utils::NodeIt scale = Parse.GetChild(node, Utils::LEVEL_ELEMENT_SCALE);
        // if (Parse.IsNodeValidInRange(scale)) {
        //     xpos = Parse.GetElement<float>(scale, Utils::LEVEL_ELEMENT_X, 0.0);
        //     info.Scale =xpos;// Vector3<float>( xpos, ypos, zpos );
        // }
        //
        Utils::NodeIt position = Parse.GetChild(node, Utils::LEVEL_ELEMENT_POSITION);
        if (Parse.IsNodeValid(position)) {
            xpos = Parse.GetElement<float>(position, Utils::LEVEL_ELEMENT_X, 0.0);
            ypos = Parse.GetElement<float>(position, Utils::LEVEL_ELEMENT_Y, 0.0);
            zpos = Parse.GetElement<float>(position, Utils::LEVEL_ELEMENT_Z, 0.0);
            info.Position = Vector3<float>( xpos, ypos, zpos );
        }

        Utils::NodeIt color = Parse.GetChild(node, Utils::LEVEL_ELEMENT_COLOR);
        if (Parse.IsNodeValid(color)) {
            xpos = Parse.GetElement<float>(color, Utils::LEVEL_ELEMENT_X, 0.0);
            ypos = Parse.GetElement<float>(color, Utils::LEVEL_ELEMENT_Y, 0.0);
            zpos = Parse.GetElement<float>(color, Utils::LEVEL_ELEMENT_Z, 0.0);
            info.Color = Vector3<float>( xpos, ypos, zpos );
        }


        Utils::NodeIt spawn = Parse.GetChild(node, Utils::LEVEL_ELEMENT_SPAWN);
        if (Parse.IsNodeValid(spawn)) {
            info.SpawnName = Parse.GetElement<std::string>(spawn, Utils::LEVEL_ELEMENT_NAME, "");
            xpos = Parse.GetElement<float>(spawn, Utils::LEVEL_ELEMENT_X, 0.0);
            ypos = Parse.GetElement<float>(spawn, Utils::LEVEL_ELEMENT_Y, 0.0);
            zpos = Parse.GetElement<float>(spawn, Utils::LEVEL_ELEMENT_Z, 0.0);
            int size = Parse.GetElement<float>(spawn, Utils::LEVEL_ELEMENT_AMOUNT, 0);
            info.Offset = Vector3<float>(xpos, ypos, zpos);
            info.Spawn = true;
        }

        Utils::NodeIt material = Parse.GetChild(node, Utils::LEVEL_ELEMENT_MATERIAL);
        if (Parse.IsNodeValid(material)) {
            info.Material = Parse.GetElement<std::string>(material, Utils::LEVEL_ELEMENT_NAME, "");
            info.Fragment = Parse.GetElement<std::string>(material, Utils::LEVEL_ELEMENT_FRAG, "");
            info.Vertex = Parse.GetElement<std::string>(material, Utils::LEVEL_ELEMENT_VERT, "");
        }

        Utils::NodeIt texture = Parse.GetChild(node, Utils::LEVEL_ELEMENT_TEXTURE);
        info.Texture = Parse.GetElement<std::string>(texture, Utils::LEVEL_ELEMENT_NAME, "");

        Utils::NodeIt model = Parse.GetChild(node, Utils::LEVEL_ELEMENT_MODEL);
        if (Parse.IsNodeValidInRange(model)) {
            info.Model = Parse.GetElement<std::string>(model, Utils::LEVEL_ELEMENT_NAME, "");
            info.IsModel = true;
        }

        Utils::NodeIt light = Parse.GetChild(node, Utils::LEVEL_ELEMENT_LIGHT);
        if (Parse.IsNodeValidInRange(light)) {
            printf("aaaaaa aaaaaa\n");
            info.Model = Parse.GetElement<std::string>(light, Utils::LEVEL_ELEMENT_NAME, "");
            info.IsLight = true;
            info.LightType = Parse.GetElement<std::string>(light, Utils::LEVEL_ELEMENT_TYPE, "");
        }

        Utils::NodeIt anim = Parse.GetChild(model, Utils::LEVEL_ELEMENT_ANIMATION);
        info.Animations.reserve(10);
        while (Parse.IsNodeValidInRange(anim)) {
            std::string animstr = Parse.GetElement<std::string>(anim, Utils::LEVEL_ELEMENT_NAME, "");
            info.Animations.push_back(animstr);
            anim = Parse.GetChild(++anim, Utils::LEVEL_ELEMENT_ANIMATION);
        }

        ParsedSceneInfo.emplace_back(info);

        node = Parse.GetChild(texture, Utils::LEVEL_ELEMENT_OBJECT);
    printf("\n-----------------PARSED--------------------------");
    printf("\n[scene]: |%s|\n[id]: %s\n[position]: [x]: %f [y]: %f [z]: %f\n[material][name]: %s [frag]: %s [vert]: %s\n[texture][name]: %s\n[model][name]: %s\n[isModel]: %d\n",
        scenename.c_str(), id.c_str(), xpos, ypos, zpos, info.Material.c_str(), info.Fragment.c_str(), info.Vertex.c_str(), info.Texture.c_str(), info.Model.c_str(), info.IsModel);
    printf("Parsed animations:\n");
    for (std::string s : info.Animations) {
            printf("%s\n", s.c_str());
        }
    printf("-------------------------------------------------\n");
    }

}
