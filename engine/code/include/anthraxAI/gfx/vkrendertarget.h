#pragma once

#include "anthraxAI/utils/defines.h"
#include "anthraxAI/utils/mathdefines.h"
#include "anthraxAI/gfx/vkdefines.h"
#include <cstdint>
#include <vulkan/vulkan_core.h>

namespace Gfx
{

#define GBUFFER_RT_SIZE 3
#define RT \
    X(RT_MAIN_COLOR, "main_color") \
    X(RT_MAIN_DEBUG, "main_debug") \
    X(RT_DEPTH, "depth") \
    X(RT_ALBEDO, "albedo") \
    X(RT_NORMAL, "normal") \
    X(RT_POSITION, "position") \
    X(RT_MASK, "mask") \
    X(RT_SHADOWS, "shadows") \
    X(RT_VISIBILITY, "visibility") \
    X(RT_SIZE, "rts size") \

#define X(element, name) element,
    typedef enum {
        RT
    } RenderTargetsList;
#undef X

    static std::string GetValue(const RenderTargetsList id)
    {
        std::string retval;
#define X(element, name) if (id == element) { retval = name; } else
    RT
#undef X
        {
            retval = "undef";
        }
        return retval;
    }
    static RenderTargetsList GetKey(const std::string& id)
    {
        RenderTargetsList retval;
#define X(element, name) if (id == name) { retval = element; } else
    RT
#undef X
        {
            retval = RT_SIZE;
        }
        return retval;
    }
    class RenderTarget
    {
        public:
            
            RenderTarget() { Name = "please dont call this ctor"; }
            RenderTarget(const std::string& name) : Name(name) {}
            RenderTarget(uint32_t id) { Name = Gfx::GetValue(static_cast<RenderTargetsList>(id)); ID = id; }
            RenderTarget(const RenderTarget& rt, uint32_t id);

            void CreateRenderTarget();
            void AllocateRTMemory();

	        void MemoryBarrier(VkCommandBuffer cmd, VkImageLayout oldlayout, VkImageLayout newlayout, int layer_count = 1);
	        void Copy(VkCommandBuffer cmd, VkBuffer buffer, uint32_t width, uint32_t height);
	        void Copy(VkCommandBuffer cmd, VkBuffer buffer,  uint32_t width, uint32_t height,VkDeviceSize layer_size, int layer_count);

            VkFormat GetFormat() { return Format; }

            void SetFormat(VkFormat format) { Format = format; }
            void SetDimensions(Vector2<int> dim) { Dimensions = dim; }
            void SetDeviceSize(VkDeviceSize dim) { DeviceSize = dim; }
            void SetDepth(bool depth) { IsDepth = depth; }
            void SetSampler(bool samp) { IsSampler = samp; }
            void SetCube(bool cube) { IsCube = cube; }

            const std::string& GetName() const { ASSERT(Name.empty(), "RenderTarget::GetName() Name is empty!"); return Name; }
            VkSampler* GetSampler() { return &Sampler; }
            VkImage GetImage() { return Image; }
            VkImageView GetImageView() { return ImageView; }
            VkDeviceMemory GetDeviceMemory() { return Memory; }
            Vector2<int> GetSize() const { return Dimensions; }
            bool IsSamplerSet() const { return IsSampler; }
            bool IsDepthSet() const { return IsDepth; }
            void Clean();

            VkDescriptorSet GetImGuiDescriptor() const { return ImGuiDescriptor; }
            void SetImGuiDescriptor(VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) { ImGuiDescriptor = ImGui_ImplVulkan_AddTexture(Sampler, ImageView, layout); }

        private:
            VkImage Image;
            VkImageView ImageView;
            VkDeviceMemory Memory;

            VkSampler Sampler;
            VkFormat Format;
            Vector2<int> Dimensions;
            VkDeviceSize DeviceSize;

            VkDescriptorSet ImGuiDescriptor;

            uint32_t ID = -1;
            bool IsSampler = false;
            bool IsDepth = false;
            bool IsStorage = false;
            bool IsCube = false;
            std::string Name;
    };
}
