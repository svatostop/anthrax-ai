#pragma once
#include "anthraxAI/utils/defines.h"
#include "anthraxAI/utils/mathdefines.h"
#include "anthraxAI/gameobjects/gameobjects.h"
#include "imgui.h"
#include <string>

#ifdef AAI_LINUX
#include <imgui_impl_x11.h>
static ImGui_ImplVulkanH_Window MainWindowData;
#endif

#include <map>
#include <functional>
#include <atomic>
namespace UI
{
#define MAX_INPUT_TEXT_SIZE 50
    enum ElementType
    {
        TEXT,
        TEXT_ENTER,
        BUTTON,
        SEPARATOR,
        TAB,
        COMBO,
        FLOAT,
        CHECKBOX,
        LISTBOX,
        TREE,
        IMAGE,
        CUBEMAP_IMAGE,
        SLIDER,
        SLIDER_3,
        DEBUG_IMAGE,
        ADD_OBJECT,
        SAVE_OBJECT,
    };

    class Element
    {
        public:

            Element(ElementType type, const std::string& label, bool isdyn = false)
            : Type(type), Label(label), IsDynamic(isdyn) { if (type == UI::TAB) { ID = IDCounter; IDCounter++;} }

            template<typename T, typename... Args>
            Element(ElementType type, const std::string& label, bool isdyn, T t, Args... args, std::function<void (std::string)> func, bool addempty = false)
            : Type(type), Label(label), IsDynamic(isdyn), Definition(func), AddEmpty(addempty) { EvaluateArgs(t, args...); }

            template<typename T, typename... Args>
            Element(ElementType type, const std::string& label, bool isdyn, T t, Args... args, std::function<void (std::string, const UI::Element& elem)> func, bool addempty = false)
            : Type(type), Label(label), IsDynamic(isdyn), DefinitionWithElem(func), AddEmpty(addempty) { EvaluateArgs(t, args...); }
 
            template<typename T, typename... Args>
            Element(ElementType type, const std::string& label, bool isdyn, T t, Args... args, std::function<void (std::string)> func, std::function<std::string ()> func_ret, bool addempty = false)
            : Type(type), Label(label), IsDynamic(isdyn), Definition(func), DefinitionString(func_ret),AddEmpty(addempty) { EvaluateArgs(t, args...); }
            
           
            template<typename T, typename... Args>
            Element(ElementType type, const std::string& label, bool isdyn, T t, Args... args, std::function<void (bool)> func)
            : Type(type), Label(label), IsDynamic(isdyn), DefinitionBool(func) { EvaluateArgs(t, args...); }

            template<typename T, typename... Args>
            Element(ElementType type, const std::string& label, bool isdyn, T t, Args... args, std::function<void (bool)> func, std::function<bool ()> funcret)
            : Type(type), Label(label), IsDynamic(isdyn), DefinitionBool(func), DefinitionBoolRet(funcret) { EvaluateArgs(t, args...); }


            Element(ElementType type, const std::string& label, bool isdyn, std::function<float (float)> func, std::function<float ()>  arg, float minmax[2])
            : Type(type), Label(label), IsDynamic(isdyn), DefinitionFloatArg(func), DefinitionFloat(arg) {  SliderMinMax[0] = minmax[0]; SliderMinMax[1] = minmax[1]; }
            
            Element(ElementType type, const std::string& label, bool isdyn, std::function<void (glm::vec3)> func, std::function<glm::vec3 ()>  arg, float minmax[2])
            : Type(type), Label(label), IsDynamic(isdyn), DefinitionFloat3Arg(func), GetDefinitionFloat3Arg(arg) { SliderMinMax[0] = minmax[0]; SliderMinMax[1] = minmax[1];  }

            Element(ElementType type, const std::string& label, bool isdyn,std::function<float ()> func)
            : Type(type), Label(label), IsDynamic(isdyn), DefinitionFloat(func) { }

            Element(ElementType type, const std::string& label, bool isdyn, std::function<std::string ()> func)
            : Type(type), Label(label), IsDynamic(isdyn), DefinitionString(func) { }
            
            Element(ElementType type, const std::string& label, bool isdyn, std::function<void (std::string)> func, std::function<std::string ()> get_func)
            : Type(type), Label(label), IsDynamic(isdyn), Definition(func), DefinitionString(get_func) { }


            int GetID() const { return ID; }
            ElementType GetType() const { return Type; }
            const std::string& GetLabel() const { return Label; }
            bool IsUIDynamic() const { return IsDynamic; }

            std::vector<std::string> GetComboList() { return ComboList;}

            std::function<void (std::string, const UI::Element& elem)> DefinitionWithElem;
            std::function<void (std::string)> Definition;
            std::function<void ()> DefinitionVoid;
            std::function<bool ()> DefinitionBoolRet;
            std::function<void (bool)> DefinitionBool;
            std::function<float (float)> DefinitionFloatArg;
            std::function<void (glm::vec3)> DefinitionFloat3Arg;
            std::function<float ()> DefinitionFloat;
            std::function<glm::vec3 ()> GetDefinitionFloat3Arg;
            std::function<std::string ()> DefinitionString;
            
            float SliderMinMax[2] = {-1.0f, 1.0f};
            inline bool operator<(const UI::Element& elem) const { return GetID() < elem.GetID(); }
            
            char InputText[MAX_INPUT_TEXT_SIZE] = {};
            int ComboInd = 0;
            bool AddEmpty = false;
            
            void ClearComboList() { ComboList.clear(); }

        private:

            void GetArg(const std::vector<std::string>& vec);
            void GetArg(void* nu) { }
            template <typename T>
            void EvaluateArgs(T t) { GetArg(t); }
            template<typename T, typename... Args>
            void EvaluateArgs(T t, Args... args) { EvaluateArgs(args...); }

            std::vector<std::string> ComboList = {};
            inline static std::atomic_int IDCounter = 0;
            int ID = 0;
            ElementType Type;
            std::string Label;
            bool IsDynamic = false;
    };

    //typedef std::map<std::string, std::vector<Element>>
    typedef std::vector<Element> UIElementsMap;
    typedef std::map<Element, std::vector<Element>> UITabsElementsMap;
    class Window
    {
        public:
            Window(const std::string& name, Vector2<float> size, Vector2<float>pos, ImGuiWindowFlags flags, bool active = true)
            : Name(name), Size(size), Position(pos), Flags(flags), Active(active) { UIElements.reserve(100); }
            Window() {}

            ImGuiWindowFlags GetFlags() const { return Flags; }
            const std::string& GetName() const { return Name; }
            bool IsActive() const { return Active; }
            void SetActive(bool active) { Active = active; }
            float GetSizeX() const { return Size.x; }
            float GetSizeY() const { return Size.y; }
            float GetPosX() const { return Position.x; }
            float GetPosY() const { return Position.y; }
            void Add(const UI::Element& element) { UIElements.push_back(element); }
            void Add(UI::Element tab, const UI::Element& element) { UITabs[tab].emplace_back(element); }
            UI::UITabsElementsMap& GetUITabs() { return UITabs; }
            UI::UIElementsMap& GetUIElements() { return UIElements; }
        private:
            Vector2<float> Size;
            Vector2<float> Position;
            ImGuiWindowFlags Flags;
            std::string Name;
            UI::UITabsElementsMap UITabs;
            UI::UIElementsMap UIElements;
            bool Active;
    };

    typedef std::map<std::string, std::vector<Window>> UIWindowsMap;
}


namespace Core
{
    class ImGuiHelper : public Utils::Singleton<ImGuiHelper>
    {
        public:
            ~ImGuiHelper();

            void DisplayObjectInfo(const std::string& obj, const UI::Element& elem);
            void UpdateObjectInfo();
            void Init();
            bool IsInit() const { return Initialized; };
            void InitUIElements();
            void Render();
            void UpdateFrame();
            void SetDebugRT(const std::string& rt) { DebugRT = rt; }

            void Add(const std::string& scene, const UI::Window& window) { UIWindows[scene].emplace_back(window); }
#ifdef AAI_LINUX
            void CatchEvent(xcb_generic_event_t *event) { ImGui_ImplX11_Event(event); }
#endif
            bool TextureNeedsUpdate() { return TextureUpdate; }
            void ResetTextureUpdate() { TextureUpdate = false;}

            struct TextureForUpdate {
                int ID;
                bool Cubemap = false;;
                std::string OldTextureName;
                std::string NewTextureName;
            };

            TextureForUpdate GetTextureForUpdate() { return TextureUpdateInfo; }
        private:
            Keeper::Objects* ParseObjectID(const std::string& id);
            void Image(UI::Element& element);
            void CubemapImage(UI::Element& element);
            void DebugImage(UI::Element& element);
            void Combo(UI::Element& element);
            void Tree(UI::Element& element);
            void ListBox(UI::Element& element);
            void ProcessUI(UI::Element& element);
            void AddObject();

            ImGuiStyle 	EditorStyle;
            ImGuiStyle 	UnhoveredStyle;
            std::string EditorName;
            std::string NewObjectNameNPC;
            std::string NewObjectNameSprite;
            std::string NewObjectNameLight;
            std::string SelectedElement;
            UI::UIWindowsMap UIWindows;
            UI::Window* Editor;

            TextureForUpdate TextureUpdateInfo;
            bool TextureUpdate = false;
            bool IsDisplayInReset = false;
            bool Initialized = false;

            std::string DebugRT;
    };
}
