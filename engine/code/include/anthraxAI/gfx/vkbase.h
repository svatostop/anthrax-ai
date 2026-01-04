#pragma once

#include "anthraxAI/utils/defines.h"
#include "anthraxAI/gfx/vkdefines.h"
#include "anthraxAI/gfx/vkdevice.h"
#include "anthraxAI/gfx/vkdebug.h"
#include "anthraxAI/gfx/vkrenderer.h"
#include "anthraxAI/gfx/vkmesh.h"
#include "anthraxAI/gfx/model.h"
#include "anthraxAI/gfx/vkdescriptors.h"
#include "anthraxAI/gfx/vkpipeline.h"
#include <vulkan/vulkan_core.h>

namespace Gfx
{
    class Vulkan : public Utils::Singleton<Vulkan>
    {
        public:
            void Init();
            void CleanUp();

            bool ReloadShaders();
            void ReloadResources();

            bool OnResize();

            VkInstance GetVkInstance() const { return Instance; }
            bool IsValidationLayersOn() const { return ValidationLayersOn; }

            void SetDebugName(const std::string& name, uint64_t handle, VkObjectType type) { Debug.SetDebugName(name, handle, type); }
            void SetRTDebugName(const std::string& name, VkImage image) {Debug.SetRTName(name, image); };
            void SetDebugName(VkDebugUtilsObjectNameInfoEXT info) { Debug.SetName(info); }
            void SetDebugRenderName(VkCommandBuffer cmd, VkDebugUtilsLabelEXT* info) { Debug.SetRenderName(cmd,info); }
            void EndDebugRenderName(VkCommandBuffer cmd) { Debug.EndRenderName(cmd); }

        private:
            VkDebug Debug;
            VkInstance Instance;

            void CreateVkInstance();
            bool EnumerateInstanceExtSupport();
            bool EnumerateValidationLayerSupport();

            bool ValidationLayersOn = true;

    };

}
