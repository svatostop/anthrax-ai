#include "anthraxAI/utils/parser.h"
#include "anthraxAI/gameobjects/gameobjects.h"
#include "anthraxAI/gameobjects/objects/light.h"
#include "anthraxAI/utils/defines.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <ostream>
#include <string>
#include <utility>

std::string Utils::Parser::ConstructElementName(const LevelElements& element) const
{
    std::string name = Utils::GetValue(element) + ":";
    return name;
}

void Utils::Parser::PrintTokenized()
{
 int i = 0;
    printf("-----------------TOKENIZED--------------------------\n");
    for (auto& it : Tokens) {
        printf("[%d][%s][%s]\n", i, it.first.c_str(), it.second.c_str());
        i++;
    }
    printf("----------------------------------------------------\n");

}

void Utils::Parser::Write(const std::string& filename)
{
    std::ofstream write;
    write.open(LEVEL_PATH + filename);

    ASSERT(!write.is_open(), ("Failed to open a file " + filename).c_str());
    
    std::string separator;
    for (auto& it : Tokens) {
        if (it.second.empty()) {
            if (it.first == Utils::GetValue(Utils::LEVEL_ELEMENT_OBJECT)) {
                separator = "";
            }
            else {
                separator = "\t";
            }
            write << separator << it.first + ":" << std::endl;
        }
        else {
            if (it.first == Utils::GetValue(Utils::LEVEL_ELEMENT_SCENE)) {
                separator = "";
            }
            else {
                separator = "\t\t";
            }
            write << separator << it.first + ": " + it.second << std::endl;
        }
    }
    write << std::endl;
    write.close();

    RootNode = Tokens.begin();
    ChildRange = Tokens.end();

    printf("File [%s] overwritten\n", filename.c_str() );
}

void Utils::Parser::Load(const std::string& filename)
{
    std::ifstream read;
    read.open(LEVEL_PATH + filename);
    ASSERT(!read.is_open(), ("Failed to open a file " + filename).c_str());

    File.reserve(256);
    std::string str, strsub;
    std::size_t foundlast = 0;

    while (std::getline(read, str)) {
        File.emplace_back(str);
    }

    read.close();
    
    Tokenize(File.begin());
    RootNode = Tokens.begin();
    ChildRange = Tokens.end();

    PrintTokenized();
}

void Utils::Parser::Tokenize(std::vector<std::string>::const_iterator it)
{
    Tokens.reserve(File.size());

    char delimeter = ':';
    std::string key;
    std::string value;
    bool delimfound = false;

    ASSERT(it == File.end(), "Utils::Parser::Tokenize() file is empty!");

    for (; it != File.end(); ++it) {
        if (*it == "\n" || (*it).empty()) continue;
        for (std::string::const_iterator ch = it->begin(); ch != it->end(); ++ch) {
            if (*ch == '\0' || *ch == ' ' || *ch == '\t' || *ch == '\n') {
                continue;
            }
            if (*ch == ':') {
                delimfound = true;
            }
            if (!delimfound) {
                key += *ch;
            }
            else if (*ch != ':') {
                value += *ch;
            }
        }
        ASSERT(!delimfound, "Utils::Parser::Tokenize() missing ':'");
        Tokens.emplace_back(std::make_pair(key, value));
        key.clear();
        value.clear();
        delimfound = false;
    }
    ASSERT(Tokens.empty(), "Utils::Parser::Tokenize() file was empty");
}

void Utils::Parser::AddToken(LevelElements element, const std::string& str)
{
    Tokens.emplace_back(std::make_pair(Utils::GetValue(element), str));
}

void Utils::Parser::ClearFile(const std::string& filename)
{
    std::ofstream fs;
    fs.open(LEVEL_PATH + filename);
    fs.close();
}

void Utils::Parser::UpdateTokens(const Keeper::Objects* obj)
{
    Keeper::Type type = obj->GetType();

    AddToken(LEVEL_ELEMENT_OBJECT, "");
    AddToken(LEVEL_ELEMENT_ID, obj->GetParsedID());
    AddToken(LEVEL_ELEMENT_POSITION, "");
    AddToken(LEVEL_ELEMENT_X, std::to_string(obj->GetPosition().x));
    AddToken(LEVEL_ELEMENT_Y, std::to_string(obj->GetPosition().y));
    AddToken(LEVEL_ELEMENT_Z, std::to_string(obj->GetPosition().z));
    AddToken(LEVEL_ELEMENT_MATERIAL, "");
    AddToken(LEVEL_ELEMENT_NAME, obj->GetMaterialName());
    AddToken(LEVEL_ELEMENT_FRAG, obj->GetFragmentName());
    AddToken(LEVEL_ELEMENT_VERT, obj->GetVertexName());
    AddToken(LEVEL_ELEMENT_TEXTURE, "");
    AddToken(LEVEL_ELEMENT_NAME, obj->GetTextureName());
    if (type == Keeper::NPC) {
        AddToken(LEVEL_ELEMENT_MODEL, "");
        AddToken(LEVEL_ELEMENT_NAME, obj->GetModelName());
        if (obj->HasAnimations()) {
            for (auto& str : obj->GetAnimations()) {
                AddToken(LEVEL_ELEMENT_ANIMATION, "");
                AddToken(LEVEL_ELEMENT_NAME, str);
            }
        }
    }
    else if (type == Keeper::LIGHT) {
        AddToken(LEVEL_ELEMENT_LIGHT, "");
        AddToken(LEVEL_ELEMENT_NAME, obj->GetModelName());
        AddToken(LEVEL_ELEMENT_COLOR, "");
        AddToken(LEVEL_ELEMENT_X, std::to_string(obj->GetColor().x));
        AddToken(LEVEL_ELEMENT_Y, std::to_string(obj->GetColor().y));
        AddToken(LEVEL_ELEMENT_Z, std::to_string(obj->GetColor().z));
    }
    RootNode = Tokens.begin();
    ChildRange = Tokens.end();
}

Utils::NodeIt Utils::Parser::GetChildByID(const NodeIt& node, const std::string& id)  const
{
    std::string key =  Utils::GetValue(Utils::LEVEL_ELEMENT_ID);

    NodeIt it = std::find_if(node, Tokens.end(), [key, id](const auto& iter) { return iter.first == key && iter.second == id; });
    if (it == Tokens.end()) {

        std::cout << ("Parser::GetChildByID(): Child not found |" + key + "| id:" + id) << std::endl;
    }
    return it;
}

Utils::NodeIt Utils::Parser::GetChild(const NodeIt& node, const LevelElements& elem) const
{
    std::string key =  Utils::GetValue(elem);
    NodeIt obj_it = Tokens.end();

    if (elem == Utils::LEVEL_ELEMENT_SCALE || elem == Utils::LEVEL_ELEMENT_ROTATION || elem == Utils::LEVEL_ELEMENT_ANIMATION || elem == Utils::LEVEL_ELEMENT_LIGHT || elem == Utils::LEVEL_ELEMENT_MODEL) {
        std::string obj_key = Utils::GetValue(Utils::LEVEL_ELEMENT_OBJECT);
        NodeIt n = node;
        if (elem != Utils::LEVEL_ELEMENT_ANIMATION ) {
            ++n;
        }
        obj_it = std::find_if(n, Tokens.end(), [obj_key](const auto& n) { return n.first == obj_key; } );
        const_cast<NodeIt&>(ChildRange) = obj_it;
    }

    NodeIt it = std::find_if(node, obj_it, [key](const auto& n) { return n.first == key; } );

    if (it == Tokens.end()) {
        std::cout << ("Parser::GetElement(): Child not found |" + key + "|") << std::endl;
    }
    return it;
}

std::string Utils::Parser::GetRootElement() const
{
    ASSERT((RootNode->first) != Utils::GetValue(Utils::LEVEL_ELEMENT_SCENE), "Parser::GetRootElement(): Root Node != 'Scene'");
    std::string root = RootNode->second;
    return root;
}
