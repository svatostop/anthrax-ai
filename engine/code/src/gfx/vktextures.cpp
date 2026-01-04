#include "anthraxAI/core/imguihelper.h"
#include "anthraxAI/gameobjects/gameobjects.h"
#include "anthraxAI/gameobjects/objects/sprite.h"
#include "anthraxAI/gfx/vkdefines.h"
#include "anthraxAI/gfx/vkrenderer.h"
#include "anthraxAI/gfx/vkdevice.h"
#include "anthraxAI/gfx/vkrendertarget.h"
#include <algorithm>
#include <cstdio>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

bool Gfx::Renderer::CreateTextureFromInfo(const std::string& texturename)
{
    if (texturename.empty()) {
        return false;
    }
    std::string path = "./textures/";
    std::string texture = texturename;

    auto it = Textures.find(texture);
    if (it != Textures.end()) {
        return false;
    }
    Textures[texture] = CreateTexture(path + texture);
    CreateSampler(Textures[texture]);

    if (Core::ImGuiHelper::GetInstance()->IsInit()) {
        Textures[texture].SetImGuiDescriptor();
    }
    return true;
}

void Gfx::Renderer::CleanTextures()
{
    for (auto& it : Textures) {
        it.second.Clean();
    }
    Textures.clear();

    for (auto& it : Cubemaps) {
        it.second.Clean();
    }
    Cubemaps.clear();
    
    for (auto& it : CubemapsImgui) {
        it.second.Clean();
    }
    CubemapsImgui.clear();

}

void Gfx::Renderer::CreateTextures()
{
    Core::Scene* scene = Core::Scene::GetInstance();
    for (auto& it : scene->GetGameObjects()->GetObjects()) {
        for (Keeper::Objects* info : it.second) {
            if (info->GetTextureName().empty()) {
                continue;
            }
            if (!CreateTextureFromInfo(info->GetTextureName())) {
                continue;
            }
        }
    }

    // load others from texture folder
    //
    std::string path = "textures/";
    std::vector<std::string> names;
    names.reserve(20);
    for (const auto& name : std::filesystem::directory_iterator(path)) {
        std::string str = name.path().string();
        std::string basename = str.substr(str.find_last_of("/\\") + 1);
        bool exists = basename.find(".jpg") != std::string::npos || basename.find(".png") != std::string::npos;
        if (exists) {
            names.emplace_back(basename.c_str());
           // printf("BASENAME: %s\n", basename.c_str());
        }
    }

    for (const std::string& s : names) {
        if (!CreateTextureFromInfo(s)) {
            continue;
        }
    }

    std::string path_cubemaps = "cubemaps/";
    std::filesystem::recursive_directory_iterator it;
    std::map<std::string, std::vector<std::string>> cb_names;
    for (const auto& dir : std::filesystem::directory_iterator(path_cubemaps)) {
           // printf("dir: %s\n", dir.path().c_str());
        if (!dir.is_directory()) continue;
    for (const auto& name : std::filesystem::directory_iterator(dir)) {
        std::string str = name.path().string();
        std::string basename = str.substr(str.find_last_of("/\\") + 1);
        bool exists = basename.find(".jpg") != std::string::npos || basename.find(".png") != std::string::npos;
        if (exists) {
            std::string dir_name =dir.path().c_str();
            cb_names[dir_name].push_back(str);
          //  printf("BASENAME: %s|%s\n", str.c_str(), dir.path().c_str());
        }
    }
    }
    
    std::vector<std::string> sorted_arr;
   // std::string faces[6] = { "front.jpg", "back.jpg", "top.jpg", "bottom.jpg", "right.jpg", "left.jpg" };
    std::string faces[6] = { "right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg"};
    std::string faces2[6] = { "px.png", "nx.png", "py.png", "ny.png", "pz.png", "nz.png"};
    for (auto& s : cb_names)
    {
        for (int i = 0; i < 6; i++) {
            auto it = std::find(s.second.begin(), s.second.end(), s.first + "/" + faces[i]);
            if (it == s.second.end()) {
                it = std::find(s.second.begin(), s.second.end(), s.first + "/" + faces2[i]);
            }
            ASSERT(it == s.second.end(), "can't find needed cubemap face!");

            sorted_arr.push_back(*it);
        }
        cb_names[s.first].clear();
        cb_names[s.first] = sorted_arr;
        sorted_arr.clear();
    }
    for (const auto& s : cb_names) {
        if (!CreateCubemaps(s.first, s.second)) {
            continue;
        }
        if (Core::ImGuiHelper::GetInstance()->IsInit()) {
            CubemapsImgui[s.first] = CreateTexture(s.second[0]);
            CreateSampler(CubemapsImgui[s.first]);
            CubemapsImgui[s.first].SetImGuiDescriptor();
        }
    }
    
}
void Gfx::Renderer::BindTextures()
{
    for (auto& it : Textures) {
        for (int i = 0 ; i < MAX_FRAMES; i++) {
            RenderTarget* rt = &(it.second);
            Gfx::DescriptorsBase::GetInstance()->UpdateTexture(rt->GetImageView(), *(rt->GetSampler()), rt->GetName(), i);
        } 
    }
    for (auto& it : Cubemaps) {
        for (int i = 0 ; i < MAX_FRAMES; i++) {
            RenderTarget* rt = &(it.second);
            Gfx::DescriptorsBase::GetInstance()->UpdateTexture(rt->GetImageView(), *(rt->GetSampler()), rt->GetName(), i);
        } 
    }
}

bool Gfx::Renderer::CreateCubemaps(const std::string& name, const std::vector<std::string>& path)
{
    if (Cubemaps.find(name) != Cubemaps.end()) {
        return false;
    }
    int width, height, channels;
    stbi_uc* pixels[6];
    for (int i = 0; i < 6; i++) {
        pixels[i] = stbi_load(path[i].c_str(), &width, &height, &channels, STBI_rgb_alpha);
        ASSERT(!pixels[i], "failed to load cubemap image!");
    }
    VkDeviceSize imagesize = width * height * 4 * 6;
    VkDeviceSize layersize = imagesize / 6; 

    BufferHelper::Buffer stagingbuffer;
    BufferHelper::AllocateBuffer(stagingbuffer, imagesize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);


    RenderTarget texture(name);
    texture.SetFormat(VK_FORMAT_R8G8B8A8_UNORM);
    texture.SetDimensions({ width, height });
    texture.SetDeviceSize(imagesize);
    texture.SetCube(true);
    texture.CreateRenderTarget();
    Submit([&](VkCommandBuffer cmd) {
        texture.MemoryBarrier(cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6);
    });

    for (int i = 0; i < 6; i++) {
        
        BufferHelper::MapMemory(stagingbuffer, layersize, layersize * i, pixels[i]);
        stbi_image_free(pixels[i]);

        Submit([&](VkCommandBuffer cmd) {
            texture.Copy(cmd, stagingbuffer.Buffer, static_cast<uint32_t>(width), static_cast<uint32_t>(height), layersize * i, i + 1);
        });
       
    } 
    Submit([&](VkCommandBuffer cmd) {
       texture.MemoryBarrier(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 6);
    });
    
    vkDestroyBuffer(Gfx::Device::GetInstance()->GetDevice(), stagingbuffer.Buffer, nullptr);
    vkFreeMemory(Gfx::Device::GetInstance()->GetDevice(), stagingbuffer.DeviceMemory, nullptr);
    
    Cubemaps[name] = texture;
    CreateSampler(Cubemaps[name]);
    return true;
}
 
Gfx::RenderTarget Gfx::Renderer::CreateTexture(const std::string& path)
{
    int width, height, channels;

    stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    VkDeviceSize imagesize = width * height * 4;

    std::cout << path << '\n';
    ASSERT(!pixels, "failed to load texture image!");

    BufferHelper::Buffer stagingbuffer;
    BufferHelper::AllocateBuffer(stagingbuffer, imagesize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    BufferHelper::MapMemory(stagingbuffer, imagesize, 0, pixels);

    stbi_image_free(pixels);

    RenderTarget texture(path);
    texture.SetFormat(VK_FORMAT_R8G8B8A8_SRGB);
    texture.SetDimensions({ width, height });

    texture.CreateRenderTarget();

    Submit([&](VkCommandBuffer cmd) {
        texture.MemoryBarrier(cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    });
    Submit([&](VkCommandBuffer cmd) {
        texture.Copy(cmd, stagingbuffer.Buffer, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    });
    Submit([&](VkCommandBuffer cmd) {
        texture.MemoryBarrier(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    });
    vkDestroyBuffer(Gfx::Device::GetInstance()->GetDevice(), stagingbuffer.Buffer, nullptr);
    vkFreeMemory(Gfx::Device::GetInstance()->GetDevice(), stagingbuffer.DeviceMemory, nullptr);

    return texture;
}

void Gfx::Renderer::CreateSampler(RenderTarget& rt)
{
    VkSamplerCreateInfo samplerinfo{};
	samplerinfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerinfo.pNext = nullptr;

	samplerinfo.magFilter = VK_FILTER_NEAREST;
	samplerinfo.minFilter = VK_FILTER_NEAREST;
	samplerinfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerinfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerinfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	VK_ASSERT(vkCreateSampler(Gfx::Device::GetInstance()->GetDevice(), &samplerinfo, nullptr, rt.GetSampler()), "failed to create sampler!");
}

void Gfx::Renderer::CreateSampler(RenderTarget* rt)
{
    VkSamplerCreateInfo samplerinfo{};
	samplerinfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerinfo.pNext = nullptr;

	samplerinfo.magFilter = VK_FILTER_NEAREST;
	samplerinfo.minFilter = VK_FILTER_NEAREST;
    if (rt == RTs[Gfx::RT_SHADOWS]) {
        samplerinfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerinfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerinfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerinfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    }
    else {
	    samplerinfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	    samplerinfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	    samplerinfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }

    rt->SetSampler(true);
	VK_ASSERT(vkCreateSampler(Gfx::Device::GetInstance()->GetDevice(), &samplerinfo, nullptr, rt->GetSampler()), "failed to create sampler!");
}
