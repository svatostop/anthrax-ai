#pragma once

#include "anthraxAI/gameobjects/gameobjects.h"
#include "anthraxAI/utils/defines.h"
#include <sstream>
#include <algorithm>
#include <string>
#include <utility>

#define LEVEL_PATH "./scenes/"

namespace Utils
{
#define LEVEL_ELEMENTS \
    X(LEVEL_ELEMENT_SCENE, "Scene") \
    X(LEVEL_ELEMENT_CUBEMAPS, "Cubemaps") \
    X(LEVEL_ELEMENT_SHADOWS, "Shadows") \
    X(LEVEL_ELEMENT_OBJECT, "Object") \
    X(LEVEL_ELEMENT_POSITION, "Position") \
    X(LEVEL_ELEMENT_ROTATION, "Rotation") \
    X(LEVEL_ELEMENT_SCALE, "Scale") \
    X(LEVEL_ELEMENT_COLOR, "Color") \
    X(LEVEL_ELEMENT_RADIUS, "Radius") \
    X(LEVEL_ELEMENT_SPAWN, "Spawn") \
    X(LEVEL_ELEMENT_AMOUNT, "Amount") \
    X(LEVEL_ELEMENT_ID, "ID") \
    X(LEVEL_ELEMENT_MATERIAL, "Material") \
    X(LEVEL_ELEMENT_TEXTURE, "Texture") \
    X(LEVEL_ELEMENT_ANIMATION, "Animation") \
    X(LEVEL_ELEMENT_MODEL, "Model") \
    X(LEVEL_ELEMENT_LIGHT, "Light") \
    X(LEVEL_ELEMENT_NAME, "name") \
    X(LEVEL_ELEMENT_FRAG, "frag") \
    X(LEVEL_ELEMENT_VERT, "vert") \
    X(LEVEL_ELEMENT_TYPE, "type") \
    X(LEVEL_ELEMENT_X, "x") \
    X(LEVEL_ELEMENT_Y, "y") \
    X(LEVEL_ELEMENT_Z, "z") \
    X(LEVEL_ELEMENT_UNDEF, "undef") \

#define X(element, name) element,
    typedef enum {
        LEVEL_ELEMENTS
    } LevelElements;
#undef X

    static std::string GetValue(const LevelElements& id)
    {
        std::string retval;
#define X(element, name) if (id == element) { retval = name; } else
    LEVEL_ELEMENTS
#undef X
        {
            retval = "undef";
        }
        return retval;
    }

    typedef std::vector<std::pair<std::string, std::string>> TokensPair;
    typedef std::vector<std::pair<std::string, std::string>>::const_iterator NodeIt;
    typedef std::vector<std::pair<std::string, std::string>>::iterator NotConstNodeIt;
    class Parser
    {
        public:
            void Load(const std::string& filename);
            void Write(const std::string& filename);
            NodeIt GetChild(const NodeIt& node, const LevelElements& elem)  const;
            NodeIt GetChildByID(const NodeIt& node, const std::string& id)  const;
            template<typename T>
            T GetElement(const NodeIt& node, const LevelElements& element, T defval) const;

            bool IsNodeValid(NodeIt& it) const { return it != Tokens.end(); }
            bool IsNodeValidInRange(NodeIt& it) const { return it != ChildRange; }
            std::string GetRootElement() const;
            NodeIt GetRootNode() const { return RootNode; }

            void Clear() { File.clear(); Tokens.clear();  }
            void UpdateElement(NodeIt it, const std::string& s) { const_cast<std::pair<std::string, std::string>&>(*it).second = s; }
            void PrintTokenized();
            void UpdateTokens(const Keeper::Objects* obj);
        
            void ClearFile(const std::string& filename);
            void AddRoot(LevelElements element,  const std::string& str) { AddToken(element, str); }
        private:
            std::vector<std::string> File;
            TokensPair Tokens;

            NodeIt RootNode;
            NodeIt ChildRange;
            NodeIt RootChildNode;
            NodeIt ChildNode;

            void Tokenize(std::vector<std::string>::const_iterator it) ;
            std::string ConstructElementName(const LevelElements& element) const;
            void AddToken(LevelElements element,  const std::string& str);
    };

    template<typename T>
    T Parser::GetElement(const NodeIt& node, const LevelElements& element, T defval) const
    {
        std::string elemtstr = GetValue(element);
        std::string value = "0";
        bool samenode = true;

        NodeIt data = std::find_if(node, Tokens.end(), [elemtstr, node, &samenode](const auto& it) {
            if (node->first != it.first && it.second.empty()) {
                samenode = false;
            }
            return samenode && it.first == elemtstr;
        });

        if (data == Tokens.end()) {
            printf("Parser::GetElement(): element |%s| not found for |%s| node\n", elemtstr.c_str(), node->first.c_str());
            return defval;
        }
        else {
            value = data->second;
        }

        std::stringstream ss(value);
        T type;
        ss >> type;
        return type;
    }
}
