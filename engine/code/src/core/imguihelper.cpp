#include "anthraxAI/core/imguihelper.h"
#include "anthraxAI/core/audio.h"
#include "anthraxAI/core/scene.h"
#include "anthraxAI/core/windowmanager.h"
#include "anthraxAI/gamemodules/modules.h"
#include "anthraxAI/gameobjects/gameobjects.h"
#include "anthraxAI/gfx/renderhelpers.h"
#include "anthraxAI/gfx/vkrenderer.h"
#include "anthraxAI/gfx/vkdevice.h"
#include "anthraxAI/gfx/vkbase.h"
#include "anthraxAI/utils/debug.h"
#include "anthraxAI/utils/mathdefines.h"
#include "imgui.h"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iterator>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

void Core::ImGuiHelper::UpdateFrame()
{
	ImGui_ImplVulkan_NewFrame();
#ifdef AAI_LINUX
	ImGui_ImplX11_NewFrame();
#else
	ImGui_ImplWin32_NewFrame();
#endif
	ImGui::NewFrame();
}

Core::ImGuiHelper::~ImGuiHelper()
{
#if defined(AAI_LINUX)
    ImGui_ImplX11_Shutdown();
#elif defined(AAI_WINDOWS)
	ImGui_ImplWin32_Shutdown();
#endif
    ImGui::DestroyContext();
}

void Core::ImGuiHelper::Init()
{
    VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = std::size(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	VkDescriptorPool imguiPool;
	VK_ASSERT(vkCreateDescriptorPool(Gfx::Device::GetInstance()->GetDevice(), &pool_info, nullptr, &imguiPool), "failed to creat imgui descriptor set!");

	ImGui::CreateContext();
#if defined(AAI_LINUX)
    ImGui_ImplX11_Init(Core::WindowManager::GetInstance()->GetConnection(), Core::WindowManager::GetInstance()->GetWindow());
#elif defined(AAI_WINDOWS)
	ImGui_ImplWin32_Init(Core::WindowManager::GetInstance()->GetWinWindow());
#endif

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::StyleColorsDark();
    io.Fonts->AddFontDefault();
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = Gfx::Vulkan::GetInstance()->GetVkInstance();
	init_info.PhysicalDevice =  Gfx::Device::GetInstance()->GetPhysicalDevice();
	init_info.Device = Gfx::Device::GetInstance()->GetDevice();
	init_info.Queue = Gfx::Device::GetInstance()->GetQueue(Gfx::GRAPHICS_QUEUE);
	init_info.DescriptorPool = imguiPool;
	init_info.MinImageCount = 2;
	init_info.ImageCount = 3;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&init_info, nullptr);

    Gfx::Renderer::GetInstance()->Submit([&](VkCommandBuffer cmd) {
        ImGui_ImplVulkan_CreateFontsTexture(cmd);
    });

	ImGui_ImplVulkan_DestroyFontUploadObjects();

    Core::Deletor::GetInstance()->Push(Core::Deletor::Type::NONE, [=, this]() {
        vkDestroyDescriptorPool(Gfx::Device::GetInstance()->GetDevice(), imguiPool, nullptr);
		ImGui_ImplVulkan_Shutdown();
	});

	ImGuiStyle& style = ImGui::GetStyle();
	style.Alpha = 1.0;
	style.WindowRounding = 3;
	style.GrabRounding = 1;
	style.GrabMinSize = 20;
	style.FrameRounding = 3;
    style.WindowBorderSize = 0;

	style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.00f, 0.40f, 0.41f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.8f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 1.00f, 1.00f, 0.65f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.44f, 0.80f, 0.80f, 0.18f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.44f, 0.80f, 0.80f, 0.27f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.44f, 0.81f, 0.86f, 0.66f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.14f, 0.18f, 0.21f, 0.73f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.54f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 1.00f, 1.00f, 0.27f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.20f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.22f, 0.29f, 0.30f, 0.71f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.00f, 1.00f, 1.00f, 0.44f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.00f, 1.00f, 1.00f, 0.74f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 1.00f, 1.00f, 0.68f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.00f, 1.00f, 1.00f, 0.36f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.00f, 1.00f, 1.00f, 0.76f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.00f, 0.65f, 0.65f, 0.46f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.01f, 1.00f, 1.00f, 0.43f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.00f, 1.00f, 1.00f, 0.62f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.00f, 1.00f, 1.00f, 0.33f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 1.00f, 1.00f, 0.42f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.00f, 1.00f, 1.00f, 0.54f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 1.00f, 1.00f, 0.54f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.00f, 1.00f, 1.00f, 0.74f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 1.00f, 1.00f, 0.22f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.00f, 0.60f, 0.61f, 0.80f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.40f, 0.41f, 0.40f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.40f, 0.41f, 1.00f);

	EditorStyle = style;
    
    ImGuiStyle style2 = style;
	style2.Colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.0f);
    UnhoveredStyle = style2;
    InitUIElements();

    Initialized = true;

    Gfx::Renderer::GetInstance()->CreateImGuiDescSet();
}

void UI::Element::GetArg(const std::vector<std::string>& vec)
{
    if (!ComboList.empty()) {
        ComboList.clear();
    }
    ComboList.reserve(vec.size() + 1);
    if (AddEmpty) {
        ComboList.emplace_back("none");
    }
    for (const std::string& s : vec) {
        ComboList.emplace_back(s);
    }
}

void Core::ImGuiHelper::InitUIElements()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 pos = viewport->Pos;
        EditorName = "Engine ;p";
        NewObjectNameNPC = "NPC Object";
        NewObjectNameSprite = "Sprite Object";
        NewObjectNameLight = "Light Object";
        Add(EditorName, UI::Window(EditorName, { 400.0f, Core::WindowManager::GetInstance()->GetScreenResolution().y - 40.0f }, { pos.x, pos.y + 40.0f }, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings ));
        Add(EditorName, UI::Window(NewObjectNameNPC, { 400.0f, 600.0f }, { 400.0f, pos.y + 40.0f }, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings, false));
        Add(EditorName, UI::Window(NewObjectNameSprite, { 400.0f, 600.0f }, { 400.0f, pos.y + 40.0f }, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings, false));
        Add(EditorName, UI::Window(NewObjectNameLight, { 400.0f, 600.0f }, { 400.0f, pos.y + 40.0f }, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings, false));

        std::string s = EditorName;
        auto it = std::find_if(UIWindows[EditorName].begin(), UIWindows[EditorName].end(), [s](const UI::Window& win) { return win.GetName() == s; });
        Editor = &*it;

    {
        std::string tablabel = "Editor";
        UI::Element tab(UI::TAB, tablabel);
        it->Add(tab, UI::Element(UI::COMBO, "Scenes", false, Core::Scene::GetInstance()->GetSceneNames(), [](std::string tag) -> void { Core::Scene::GetInstance()->SetCurrentScene(tag); }, true));
        it->Add(tab, UI::Element(UI::SEPARATOR, "tabseparator"));
        it->Add(UI::Element(UI::BUTTON, "New Scene", false, []() -> float { Core::Scene::GetInstance()->NewScene(); return 0.0f; }));

        it->Add(UI::Element(UI::SEPARATOR, "sepa"));
        it->Add(UI::Element(UI::BUTTON, "Save Scene", false, []() -> float { Core::Scene::GetInstance()->ExportScene(); return 0.0f; }));
        it->Add(tab, UI::Element(UI::SEPARATOR, "tabseparator"));
        it->Add(UI::Element(UI::BUTTON, "Update Shaders", false, []() -> float { Gfx::Vulkan::GetInstance()->ReloadShaders(); return 0.0f; }));
        it->Add(UI::Element(UI::CHECKBOX, "Keep Editor", false, nullptr, [](bool show) -> void {  Core::Scene::GetInstance()->KeepEditor(show); }, []() -> bool {  return Core::Scene::GetInstance()->GetKeepEditor(); }));
    }
    {
        UI::Element scenetab(UI::TAB, "Scene");
        Editor->Add(scenetab, UI::Element(UI::TEXT_ENTER, "Name", false,  [](std::string name) -> void { Core::Scene::GetInstance()->SetCurrentSceneForUpdate(name); }, []() -> std::string { return Core::Scene::GetInstance()->GetCurrentSceneForUpdate(); }));
        // Editor->Add(scenetab, UI::Element(UI::SEPARATOR, "sep"));
    }

    {
        UI::Element rendertab(UI::TAB, "Rendering");
        it->Add(rendertab, UI::Element(UI::COMBO, "Render Targets", false, Gfx::Renderer::GetInstance()->GetRTList(), [](std::string tag) -> void { ImGuiHelper::GetInstance()->SetDebugRT(tag); }, true));
        it->Add(rendertab, UI::Element(UI::DEBUG_IMAGE, "image", false));
        it->Add(rendertab, UI::Element(UI::SEPARATOR, "sep"));
        it->Add(rendertab, UI::Element(UI::TEXT, "Lighting"));
        it->Add(rendertab, UI::Element(UI::SEPARATOR, "sep"));
        it->Add(rendertab, UI::Element(UI::CHECKBOX, "Shadows", false, nullptr, [](bool show) -> void { Core::Scene::GetInstance()->SetShadows(show); }, []() -> bool { return Core::Scene::GetInstance()->GetShadows(); }));
        it->Add(rendertab, UI::Element(UI::SEPARATOR, "sep"));
        float minmax[2] = {-50.0f,50.0f};
        it->Add(rendertab, UI::Element(UI::SLIDER_3, "Global Light Dir", false, [](glm::vec3 v) -> void { Gfx::Renderer::GetInstance()->SetGlobalLightDir(v); }, []() -> glm::vec3 { return Gfx::Renderer::GetInstance()->GetGlobalLightDir(); }, minmax ));
        it->Add(rendertab, UI::Element(UI::SLIDER_3, "Ambient", false, [](glm::vec3 v) -> void { Gfx::Renderer::GetInstance()->SetAmbient(v); }, []() -> glm::vec3 { return Gfx::Renderer::GetInstance()->GetAmbient(); } , minmax ));
        it->Add(rendertab, UI::Element(UI::SLIDER_3, "Specular", false, [](glm::vec3 v) -> void { Gfx::Renderer::GetInstance()->SetSpecular(v); }, []() -> glm::vec3 { return Gfx::Renderer::GetInstance()->GetSpecular(); } , minmax ));
        it->Add(rendertab, UI::Element(UI::SLIDER_3, "Diffuse", false, [](glm::vec3 v) -> void { Gfx::Renderer::GetInstance()->SetDiffuse(v); }, []() -> glm::vec3 { return Gfx::Renderer::GetInstance()->GetDiffuse(); } , minmax ));

        it->Add(rendertab, UI::Element(UI::SEPARATOR, "sep"));
        it->Add(rendertab, UI::Element(UI::CHECKBOX, "Cubemap", false, nullptr, [](bool show) -> void { Gfx::Renderer::GetInstance()->SetCubemapRendering(show); }, []() -> bool { return Gfx::Renderer::GetInstance()->GetCubemapRendering(); }));
        it->Add(rendertab, UI::Element(UI::CUBEMAP_IMAGE, "maps", false));
        it->Add(rendertab, UI::Element(UI::SEPARATOR, "sep"));
        it->Add(rendertab, UI::Element(UI::CHECKBOX, "Particles", false, nullptr, [](bool show) -> void { Core::Scene::GetInstance()->SetParticles(show); }, []() -> bool { return Core::Scene::GetInstance()->GetParticles(); }));
        it->Add(rendertab, UI::Element(UI::SEPARATOR, "sep"));
    }   

    {
        UI::Element audiotab(UI::TAB, "Audio");
        it->Add(audiotab, UI::Element(UI::COMBO, "Sounds", false, Core::Audio::GetInstance()->GetAudioNames(), [](std::string tag) -> void { Core::Audio::GetInstance()->Load(tag); }, true));
        it->Add(audiotab, UI::Element(UI::SEPARATOR, "sep"));
        it->Add(audiotab, UI::Element(UI::TEXT, "Current Sound:", false, []() -> std::string { return Core::Audio::GetInstance()->GetCurrentSound(); } ));
        it->Add(audiotab, UI::Element(UI::SEPARATOR, "sep"));
        it->Add(audiotab, UI::Element(UI::CHECKBOX, "play", false, nullptr, [](bool visible) -> void {  Core::Audio::GetInstance()->SetState(visible); }, []() -> bool {return Core::Audio::GetInstance()->GetState(); }));
        it->Add(audiotab, UI::Element(UI::SLIDER, "volume", false, [](float volume) -> float { Core::Audio::GetInstance()->SetVolume(volume); return 0.0f; }, []() -> float { return Core::Audio::GetInstance()->GetVolume(); } ));
    }

    {
        UI::Element debugtab(UI::TAB, "Debug");
        it->Add(debugtab, UI::Element(UI::TEXT, "This is debug tab"));
        it->Add(debugtab, UI::Element(UI::SEPARATOR, "sep"));
        it->Add(debugtab, UI::Element(UI::FLOAT, "fps", false, []() -> float { return Utils::Debug::GetInstance()->FPS; }));
        it->Add(debugtab, UI::Element(UI::SEPARATOR, "sep"));
        it->Add(debugtab, UI::Element(UI::CHECKBOX, "3d grid", false, nullptr, [](bool visible) -> void {  Utils::Debug::GetInstance()->Grid = visible; }, []() -> bool { return Utils::Debug::GetInstance()->Grid ; }));
        it->Add(debugtab, UI::Element(UI::CHECKBOX, "show bones weight", false, nullptr, [](bool show) -> void {  Utils::Debug::GetInstance()->Bones = show; }, []() -> bool { return Utils::Debug::GetInstance()->Bones; }));
        it->Add(debugtab, UI::Element(UI::TEXT, "Threads and info"));
        it->Add(debugtab, UI::Element(UI::SEPARATOR, "sep"));
        it->Add(debugtab, UI::Element(UI::TEXT, "draw calls:", false, []() -> std::string { return Utils::Debug::GetInstance()->GetDrawCalls(); }));
        it->Add(debugtab, UI::Element(UI::SEPARATOR, "sep"));
        it->Add(debugtab, UI::Element(UI::TEXT, "thread:", false, []() -> std::string { return Thread::Pool::GetInstance()->Time.GetTime(Thread::Task::Name::UPDATE);}));
        it->Add(debugtab, UI::Element(UI::SEPARATOR, "sep"));
    }
    {
        std::string s1 = NewObjectNameNPC;
        auto it_obj = std::find_if(UIWindows[EditorName].begin(), UIWindows[EditorName].end(), [s1](const UI::Window& win) { return win.GetName() == s1; });

        it_obj->Add(UI::Element(UI::TEXT_ENTER, "Tag", false, [](std::string tag) -> void { Scene::GetInstance()->SetNewObjectParsedID(tag); }, []() -> std::string { return Scene::GetInstance()->GetNewObjectParsedID(); } ));
        float minmax[2] = {-1000.0, 1000.0};
        it_obj->Add(UI::Element(UI::SLIDER_3, "Position", false, [](glm::vec3 v) -> void { Scene::GetInstance()->SetNewObjectPosition(v); }, []() -> glm::vec3 { return Scene::GetInstance()->GetNewObjectPosition(); }, minmax));
        it_obj->Add(UI::Element(UI::IMAGE, "Textures", false, [](std::string tag) -> void { Scene::GetInstance()->SetNewObjectTexture(tag); }, []() -> std::string { return Scene::GetInstance()->GetNewObjectTexture(); } ));
        it_obj->Add(UI::Element(UI::SEPARATOR, "sep", false));
        it_obj->Add(UI::Element(UI::COMBO, "Models", false, Gfx::Model::GetInstance()->GetModelNames(), [](std::string tag) -> void { Scene::GetInstance()->SetNewObjectModel(tag); }, []() -> std::string { return Scene::GetInstance()->GetNewObjectModel(); }, true));
        it_obj->Add(UI::Element(UI::COMBO, "Material", false, Gfx::Pipeline::GetInstance()->GetMaterialNames(), [](std::string tag) -> void { Scene::GetInstance()->SetNewObjectMaterial(tag); }, []() -> std::string { return Scene::GetInstance()->GetNewObjectMaterial(); }, true));
        it_obj->Add(UI::Element(UI::SAVE_OBJECT, "Save", false));
    }
    {
        std::string s1 = NewObjectNameSprite;
        auto it_obj = std::find_if(UIWindows[EditorName].begin(), UIWindows[EditorName].end(), [s1](const UI::Window& win) { return win.GetName() == s1; });

        it_obj->Add(UI::Element(UI::TEXT_ENTER, "Tag", false, [](std::string tag) -> void { Scene::GetInstance()->SetNewObjectParsedID(tag); }, []() -> std::string { return Scene::GetInstance()->GetNewObjectParsedID(); } ));
        float minmax[2] = {-1000.0, 1000.0};
        it_obj->Add(UI::Element(UI::SLIDER_3, "Position", false, [](glm::vec3 v) -> void { Scene::GetInstance()->SetNewObjectPosition(v); }, []() -> glm::vec3 { return Scene::GetInstance()->GetNewObjectPosition(); }, minmax));
        it_obj->Add(UI::Element(UI::IMAGE, "Textures", false, [](std::string tag) -> void { Scene::GetInstance()->SetNewObjectTexture(tag); }, []() -> std::string { return Scene::GetInstance()->GetNewObjectTexture(); } ));
        it_obj->Add(UI::Element(UI::SEPARATOR, "sep", false));
        it_obj->Add(UI::Element(UI::COMBO, "Material", false, Gfx::Pipeline::GetInstance()->GetMaterialNames(), [](std::string tag) -> void { Scene::GetInstance()->SetNewObjectMaterial(tag); }, []() -> std::string { return Scene::GetInstance()->GetNewObjectMaterial(); }, true));
        it_obj->Add(UI::Element(UI::SAVE_OBJECT, "Save", false));
    }
    {
        std::string s1 = NewObjectNameLight;
        auto it_obj = std::find_if(UIWindows[EditorName].begin(), UIWindows[EditorName].end(), [s1](const UI::Window& win) { return win.GetName() == s1; });

        it_obj->Add(UI::Element(UI::TEXT_ENTER, "Tag", false, [](std::string tag) -> void { Scene::GetInstance()->SetNewObjectParsedID(tag); }, []() -> std::string { return Scene::GetInstance()->GetNewObjectParsedID(); } ));
        float minmax[2] = {-1000.0, 1000.0};
        float minmaxcolor[2] = {0.0, 1.0};
        std::vector<std::string> types = {"point"};
        it_obj->Add(UI::Element(UI::COMBO, "Type", false, types, [](std::string tag) -> void { Scene::GetInstance()->SetNewObjectLightType(tag); }, false));
        it_obj->Add(UI::Element(UI::SLIDER_3, "Position", false, [](glm::vec3 v) -> void { Scene::GetInstance()->SetNewObjectPosition(v); }, []() -> glm::vec3 { return Scene::GetInstance()->GetNewObjectPosition(); }, minmax));
        it_obj->Add(UI::Element(UI::SLIDER_3, "Color", false, [](glm::vec3 v) -> void { Scene::GetInstance()->SetNewObjectColor(v); }, []() -> glm::vec3 { return Scene::GetInstance()->GetNewObjectColor(); }, minmaxcolor));
        it_obj->Add(UI::Element(UI::SEPARATOR, "sep", false));
        it_obj->Add(UI::Element(UI::COMBO, "Models", false, Gfx::Model::GetInstance()->GetModelNames(), [](std::string tag) -> void { Scene::GetInstance()->SetNewObjectModel(tag); }, []() -> std::string { return Scene::GetInstance()->GetNewObjectModel(); }, true));
        it_obj->Add(UI::Element(UI::COMBO, "Material", false, Gfx::Pipeline::GetInstance()->GetMaterialNames(), [](std::string tag) -> void { Scene::GetInstance()->SetNewObjectMaterial(tag); }, []() -> std::string { return Scene::GetInstance()->GetNewObjectMaterial(); }, true));
        it_obj->Add(UI::Element(UI::SAVE_OBJECT, "Save", false));
    }

}

Keeper::Objects* Core::ImGuiHelper::ParseObjectID(const std::string& id)
{
    Keeper::Type type;
    if (id.find("Camera:") != std::string::npos) {
        type = Keeper::Type::CAMERA;
    }
    else if (id.find("NPC:") != std::string::npos) {
        type = Keeper::Type::NPC;
    }
    else if (id.find("Light:") != std::string::npos) {
        type = Keeper::Type::LIGHT;
    }
    else {
        type = Keeper::Type::SPRITE;
    }

    std::string trimstr = id;
    auto it = std::find_if(trimstr.begin(), trimstr.end(), [](char c) { return c == ':'; });

    std::string type_str(trimstr.begin(), it);

    trimstr.erase(trimstr.begin(), it + 1);
    Keeper::Objects* game_obj = nullptr;

    it = std::find_if(trimstr.begin(), trimstr.end(), [](char c) { return !(c >= '0' && c <= '9') && c != ' '; });
    if (it == trimstr.end()) {
        game_obj = const_cast<Keeper::Base*>(Core::Scene::GetInstance()->GetGameObjects())->GetNotConstObject(type, std::stoi(trimstr));
    }
    else {
        trimstr.erase(std::remove_if(trimstr.begin(), trimstr.end(), isspace));
        game_obj = const_cast<Keeper::Base*>(Core::Scene::GetInstance()->GetGameObjects())->GetNotConstObject(type, trimstr);
    }
    return game_obj;
}

void Core::ImGuiHelper::DisplayObjectInfo(const std::string& obj, const UI::Element& elem)
{
    if (Editor->GetName().empty() || !IsDisplayInReset) return;
    
    auto ui_it = std::remove_if(Editor->GetUITabs()[elem].begin(), Editor->GetUITabs()[elem].end(), [](const UI::Element& el) { return el.IsUIDynamic(); });
    Keeper::Objects* game_obj = ParseObjectID(obj);

    Editor->GetUITabs()[elem].erase(ui_it, Editor->GetUITabs()[elem].end());
    Editor->Add(elem, UI::Element(UI::TEXT, "Type: ", true));
    Editor->Add(elem, UI::Element(UI::TEXT, "Position: ", true, [game_obj]() -> std::string { return game_obj->GetPosition().ToString(); } ));
    Editor->Add(elem, UI::Element(UI::TEXT, "Texture: " + game_obj->GetTextureName(), true));
    Editor->Add(elem, UI::Element(UI::TEXT, "Model: " + game_obj->GetModelName(), true));
    std::vector<std::string> s = { "Fragment: " + game_obj->GetFragmentName(), "Vertex: " + game_obj->GetVertexName() };
    Editor->Add(elem, UI::Element(UI::TREE, "Material: " + game_obj->GetMaterialName(), true, s ,[](std::string tag) -> void { return; }, false));

    Editor->Add(elem, UI::Element(UI::SEPARATOR, "sep", true));
    if (game_obj->HasAnimations()) {
        Editor->Add(elem, UI::Element(UI::TEXT, "Animations: ", true));
        std::string anim_id = game_obj->GetParsedID();
        if (!game_obj->GetSpawnName().empty()) {
            anim_id = game_obj->GetSpawnName();
        }
        Editor->Add(elem, UI::Element(UI::COMBO, "list", true, game_obj->GetAnimations(), [anim_id](std::string tag) -> void { Core::Scene::GetInstance()->ReloadAnimation(anim_id, tag); }, false));
        Editor->Add(elem, UI::Element(UI::SEPARATOR, "sep", true));
    }

    float minmax[2] = {0.0f,180.0f};
    Editor->Add(elem, UI::Element(UI::IMAGE, "Textures", true));
    Editor->Add(elem, UI::Element(UI::SEPARATOR, "sep", true));
    Editor->Add(elem, UI::Element(UI::BUTTON, "Update Object", true, [game_obj]() -> float { Core::Scene::GetInstance()->ExportObjectInfo(game_obj); return 0.0f; }));
    Editor->Add(elem, UI::Element(UI::SEPARATOR, "sep", true));
    Editor->Add(elem, UI::Element(UI::SLIDER_3, "Rotation", true, [game_obj](glm::vec3 v) -> void { game_obj->SetRotation(v); }, [game_obj]() -> glm::vec3 { return game_obj->GetRotation(); }, minmax));
    Editor->Add(elem, UI::Element(UI::SEPARATOR, "sep", true));
    IsDisplayInReset = false;
}

void Core::ImGuiHelper::UpdateObjectInfo()
{
    SelectedElement.clear();

    IsDisplayInReset = true;
    for (auto& it : Editor->GetUITabs()) {
        if (it.first.GetLabel() == "Scene") {
            for (auto& elem : it.second) {
                elem.ClearComboList();
            }
            it.second.clear();

            Editor->Add(it.first, UI::Element(UI::TEXT_ENTER, "Name", false,  [](std::string name) -> void { Core::Scene::GetInstance()->SetCurrentSceneForUpdate(name); }, []() -> std::string { return Core::Scene::GetInstance()->GetCurrentSceneForUpdate(); }));
            //Editor->Add(it.first, UI::Element(UI::TEXT, "Name:", false, []() -> std::string { return Core::Scene::GetInstance()->GetCurrentScene(); } ));
            Editor->Add(it.first, UI::Element(UI::SEPARATOR, "sep"));
            Editor->Add(it.first, UI::Element(UI::LISTBOX, "Objects", false, Core::Scene::GetInstance()->GetGameObjects()->GetObjectNames(), [this](std::string tag, const UI::Element& elem) -> void { DisplayObjectInfo(tag, elem); }, false));
            Editor->Add(it.first, UI::Element(UI::SEPARATOR, "sep"));
            Editor->Add(it.first, UI::Element(UI::ADD_OBJECT, "Add Object", false));
            Editor->Add(it.first, UI::Element(UI::SEPARATOR, "sep"));
            break;
        }
    }
}

void Core::ImGuiHelper::Combo(UI::Element& element)
{
    std::string tag = element.GetComboList()[element.ComboInd];
    if (element.DefinitionString) {
        tag =element.DefinitionString(); 
    }
   // printf("---%s|%d|%d\n\n\n", element.GetLabel().c_str(), element.ComboInd, element.GetComboList().size());
    if (ImGui::BeginCombo(element.GetLabel().c_str(), tag.c_str(), 0)) {
        size_t size = element.GetComboList().size();
        for (int n = 0; n < size; n++) {
            const bool is_selected = (element.ComboInd == n);
            if (ImGui::Selectable(element.GetComboList()[n].c_str(), is_selected)) {
                element.ComboInd = n;
                if (element.GetComboList()[n] != "none") {
                    element.Definition(element.GetComboList()[element.ComboInd]);
                }
            }

            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

void Core::ImGuiHelper::ListBox(UI::Element& element)
{
    ImGui::Text(element.GetLabel().c_str());

    if (ImGui::BeginListBox(std::string("##" + element.GetLabel()).c_str())) {
        size_t size = element.GetComboList().size();
        for (int n = 0; n < size; n++) {
            const bool is_selected = (element.ComboInd == n);
            if (ImGui::Selectable(element.GetComboList()[n].c_str(), is_selected)) {
                element.ComboInd = n;
                if (n != 0) {
                    SelectedElement = element.GetComboList()[element.ComboInd];
                    IsDisplayInReset = true;
                }
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndListBox();
    }

}

void Core::ImGuiHelper::Tree(UI::Element& element)
{
    if (ImGui::TreeNode(element.GetLabel().c_str()))
    {
        for (const std::string& s : element.GetComboList()) {
            ImGui::Text(s.c_str());
        }

        ImGui::TreePop();
    }

}

void Core::ImGuiHelper::DebugImage(UI::Element& element)
{
    ImGui::Text(element.GetLabel().c_str());

    static bool active = true;
    if (DebugRT.empty() || DebugRT == "none") {
        return;
    }
    Gfx::RenderTargetsList id = Gfx::GetKey(DebugRT);
    Gfx::RenderTarget* rt = Gfx::Renderer::GetInstance()->GetRT(id);
    float ratio = float(rt->GetSize().x) / float(rt->GetSize().y);
    if (rt) {
        active = true;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const ImVec2 pos = viewport->Pos;
        ImGui::SetNextWindowPos(ImVec2(0.0f, viewport->GetCenter().y), 0);
        ImGui::SetNextWindowSize(ImVec2(400.0f, 400.0f / ratio), ImGuiCond_FirstUseEver);
        ImGui::Begin(DebugRT.c_str(), &active, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings );

        if (!rt->IsDepthSet()) {
            Gfx::Renderer::GetInstance()->Submit([&](VkCommandBuffer cmd) {
                rt->MemoryBarrier(cmd, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            });
        }

        ImGui::Image((ImTextureID)rt->GetImGuiDescriptor(), ImVec2(400.0f, 400.0f / ratio));
        ImGui::End();

        if (!active) {
            DebugRT = "none";
        }
    }
}
void Core::ImGuiHelper::CubemapImage(UI::Element& element)
{
    ImGui::Text(element.GetLabel().c_str());
                
    if (element.DefinitionString) {
        ImGui::Text("Current texture: %s", element.DefinitionString().c_str());
    }
    for (auto& it : Gfx::Renderer::GetInstance()->GetCubemapImguiMap()) {
        Gfx::RenderTarget* rt = Gfx::Renderer::GetInstance()->GetCubemapImgui(it.first);
        if (ImGui::ImageButton((ImTextureID)rt->GetImGuiDescriptor(), ImVec2(50, 50))) {
            if (element.DefinitionString && element.Definition) {
                element.Definition(it.first);
            }
            else {
                TextureUpdateInfo.OldTextureName = Core::Scene::GetInstance()->GetModule("lighting").GetRenderQueue(Modules::RQ_GENERAL)[0].TextureName;// obj->GetTextureName();
                Core::Scene::GetInstance()->GetModule("lighting").GetRenderQueue(Modules::RQ_GENERAL)[0].TextureName = it.first;
                TextureUpdateInfo.NewTextureName = it.first;
                TextureUpdateInfo.Cubemap = true;;
                printf("INFO TEXTURE: new ->%s !!! old->%s\n", TextureUpdateInfo.NewTextureName.c_str(), TextureUpdateInfo.OldTextureName.c_str());
                TextureUpdate = true;
            }
        }
        ImGui::SameLine();
    }
}

void Core::ImGuiHelper::Image(UI::Element& element)
{
    ImGui::Text(element.GetLabel().c_str());
                
    if (element.DefinitionString) {
        ImGui::Text("Current texture: %s", element.DefinitionString().c_str());
    }
    for (auto& it : Gfx::Renderer::GetInstance()->GetTextureMap()) {
        Gfx::RenderTarget* rt = Gfx::Renderer::GetInstance()->GetTexture(it.first);
        if (ImGui::ImageButton((ImTextureID)rt->GetImGuiDescriptor(), ImVec2(50, 50))) {
            if (element.DefinitionString && element.Definition) {
                element.Definition(it.first);
            }
            else {
                TextureUpdateInfo.Cubemap = false;
                Keeper::Objects* obj = ParseObjectID(SelectedElement);
                TextureUpdateInfo.OldTextureName = obj->GetTextureName();
                obj->SetTextureName(it.first);
                TextureUpdateInfo.NewTextureName = it.first;
                TextureUpdateInfo.ID = obj->GetID();
                IsDisplayInReset = true;
                printf("OBJECT [%d] INFO TEXTURE: new ->%s !!! old->%s\n",obj->GetID(), TextureUpdateInfo.NewTextureName.c_str(), TextureUpdateInfo.OldTextureName.c_str());
                TextureUpdate = true;
            }
        }
        ImGui::SameLine();
    }
}

void Core::ImGuiHelper::AddObject()
{
    //it_obj->Add(UI::Element(UI::COMBO, "Type", false, Core::Scene::GetInstance()->GetGameObjects()->GetObjectTypes(), [](std::string tag) -> void { Scene::GetInstance()->SetNewObjectType(tag); }));
    //static UI::Element select_type = UI::Element(UI::COMBO, "Type", false, Core::Scene::GetInstance()->GetGameObjects()->GetObjectTypes(), [](std::string tag) -> void { Scene::GetInstance()->SetNewObjectType(tag); });
    
    std::string s;
    if (Scene::GetInstance()->GetNewObjectType() == "NPC") {
        s = NewObjectNameNPC;
    }
    if (Scene::GetInstance()->GetNewObjectType() == "Light") {
        s = NewObjectNameLight;
    }
    if (Scene::GetInstance()->GetNewObjectType() == "Sprite") {
        s = NewObjectNameSprite;
    }
    if (s.empty()) {
        return;
    }
    auto it = std::find_if(UIWindows[EditorName].begin(), UIWindows[EditorName].end(), [s](const UI::Window& win) { return win.GetName() == s; });
    
//    static bool test = false;
    if (it != UIWindows[EditorName].end() && !it->IsActive()) {
        it->SetActive(true);
        Scene::GetInstance()->ClearNewObjectInfo();
        for (auto& it_close : UIWindows[EditorName]) {
            if (it_close.GetName() == EditorName) continue;

            if (it_close.GetName() != it->GetName()) {
                it_close.SetActive(false);
            }
        }
    }

}

void Core::ImGuiHelper::ProcessUI(UI::Element& element)
{
    switch (element.GetType()) {
        case UI::TREE: {
            Tree(element);
            break;
        }
        case UI::CUBEMAP_IMAGE: {
            CubemapImage(element);
            break;
        }
        case UI::IMAGE: {
            Image(element);
            break;
        }
        case UI::DEBUG_IMAGE: {
            DebugImage(element);
            break;
        }
        case UI::COMBO: {
            Combo(element);
            break;
        }
        case UI::LISTBOX: {
            ListBox(element);
            break;
        }
        case UI::FLOAT: {
            ImGui::Text((element.GetLabel() + ": %f").c_str(), element.DefinitionFloat());
            break;
        }
        case UI::BUTTON: {
            if (ImGui::Button(element.GetLabel().c_str())) {
                if (element.DefinitionFloat) {
                    element.DefinitionFloat();
                }
            }
            break;
        }
        case UI::ADD_OBJECT: {
            bool test = false;
            if (ImGui::Button(element.GetLabel().c_str())) {
                ImGui::OpenPopup("my_select_popup");
            }
            if (ImGui::BeginPopup("my_select_popup"))
            {
                ImGui::SeparatorText("Select Type:");
                for (int i = 0; i < Core::Scene::GetInstance()->GetGameObjects()->GetObjectTypes().size(); i++) {
                    if (ImGui::Selectable(Core::Scene::GetInstance()->GetGameObjects()->GetObjectTypes()[i].c_str())) {
                        Scene::GetInstance()->SetNewObjectType(Core::Scene::GetInstance()->GetGameObjects()->GetObjectTypes()[i]);
                        test = true;
                    }
                }
                ImGui::EndPopup();
            }
            if (test) {
                AddObject();
            }
            break;
        }
        case UI::SAVE_OBJECT: {
            if (ImGui::Button(element.GetLabel().c_str())) {
                Core::Scene::GetInstance()->SaveObject(); 
            }
            break;
        }
        case UI::CHECKBOX: {
            if (element.DefinitionBoolRet) {
                bool check = element.DefinitionBoolRet();
                ImGui::Checkbox(element.GetLabel().c_str(), &check);
                element.DefinitionBool(check);
            }
            break;
        }
        case UI::TEXT: {
            if (element.DefinitionString) {
                ImGui::Text((element.GetLabel() + ": %s").c_str(), element.DefinitionString().c_str());
            }
            else {
                ImGui::TextUnformatted(element.GetLabel().c_str());
            }
            break;
        }
        case UI::TEXT_ENTER: {
            if (element.DefinitionString) {
                if (!element.DefinitionString().empty()) {
                    strcpy(element.InputText, element.DefinitionString().c_str());
                }
                ImGui::InputText(element.GetLabel().c_str(), element.InputText, MAX_INPUT_TEXT_SIZE);
                if (strlen(element.InputText) != 0) {
                    element.Definition(element.InputText);
                }
            }
            break;
        }
        case UI::SEPARATOR: {
            ImGui::Separator();
            break;
        }
        case UI::SLIDER: {
            float arg = element.DefinitionFloat();
            ImGui::SliderFloat(element.GetLabel().c_str(), &arg, 0.0f, 1.0f);
            if (element.DefinitionFloatArg) {
                element.DefinitionFloatArg(arg);
            }
            break;
        }
        case UI::SLIDER_3: {
            glm::vec3 arg(0.0); 
            if (element.GetDefinitionFloat3Arg) {
                arg = element.GetDefinitionFloat3Arg();
            }
            float v[3] = { arg.x, arg.y, arg.z };
            ImGui::SliderFloat3(element.GetLabel().c_str(), v, element.SliderMinMax[0], element.SliderMinMax[1], "%.3f");
            if (element.DefinitionFloat3Arg) {
                element.DefinitionFloat3Arg(glm::vec3(v[0], v[1], v[2]));
            }
            break;
        }
        default:
            break;
    }
}

void Core::ImGuiHelper::Render()
{
  //  ImGui::ShowDemoWindow();
    
    std::vector<UI::Window>& windows = UIWindows[EditorName];

    for (UI::Window& window : windows) {
        bool active = true;
        if (!window.IsActive()) continue;
        
	    ImGuiStyle& style = ImGui::GetStyle();
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow)) { 
            style = EditorStyle;
        }
        else {
            style = UnhoveredStyle;
        }

        ImGui::SetNextWindowPos(ImVec2(window.GetPosX(), window.GetPosY()), 0);
        ImGui::SetNextWindowSize(ImVec2(window.GetSizeX(), window.GetSizeY()), ImGuiCond_FirstUseEver);
        
        if (window.GetName() != EditorName) {
            active = window.IsActive();
        }
        ImGui::Begin(window.GetName().c_str(), &active, window.GetFlags());
        UI::UITabsElementsMap& uitabs = window.GetUITabs();
        for (auto& it : uitabs) {
            std::vector<UI::Element>& tabsui = it.second;
            UI::Element tab = it.first;

            if (ImGui::BeginTabBar("Tabs", ImGuiTabBarFlags_None)){
                if (ImGui::BeginTabItem(tab.GetLabel().c_str())) {
                    for (UI::Element& element : tabsui) {
                        ProcessUI(element);
                    }
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }
        if (window.GetName() != EditorName) {
            window.SetActive(active);
        }

        std::vector<UI::Element>& windowelements = window.GetUIElements();
        for (UI::Element& element : windowelements) {
            ProcessUI(element);
        }

        ImGui::End();
    }
    if (!SelectedElement.empty()) {
        auto it = std::find_if(Editor->GetUITabs().begin(), Editor->GetUITabs().end(), [](std::pair<UI::Element, std::vector<UI::Element>> pair) { return pair.first.GetLabel() == "Scene"; } );
        DisplayObjectInfo(SelectedElement, it->first);
    }
}
