module;
#include "aai/gfx/vk/backend/vk_defines.h"

export module aai.gfx.vk.rt;
import aai.gfx.vk.device;
import aai.utils;
import std;
import glm;
export {
   namespace rt {
       class render_target {
           public:
                render_target() { name = "please dont call this ctor"; }
                render_target(const std::string& n) : name(n) {}
                //render_target(uint32_t id) { name = Gfx::GetValue(static_cast<RenderTargetsList>(id)); ID = id; }
                //render_target(const RenderTarget& rt, uint32_t id);

                void create(const vk::device::handlers& dev);
                void memory_barrier(VkCommandBuffer cmd, VkImageLayout oldlayout, VkImageLayout newlayout, int layer_count = 1);
	            void copy(VkCommandBuffer cmd, VkBuffer buffer,  uint32_t width, uint32_t height, int layer_count = 1);

                void set_format(VkFormat form) { format = form; }
                void set_dimensions(const glm::vec2& dim) { dimensions = dim; }
                void set_device_size(VkDeviceSize dim) { device_size = dim; }
                void set_depth(bool depth) { is_depth = depth; }
                void set_sampler(bool samp) { is_sampler = samp; }
                void set_cube(bool cube) { is_cube = cube; }

                const std::string& get_name() const { utils::ASSERT(name.empty(), "RenderTarget::GetName() Name is empty!"); return name; }
                VkFormat get_format() { return format; }
                VkSampler* get_sampler() { return &sampler; }
                VkImage get_image() { return image; }
                VkImageView get_image_view() { return image_view; }
                VkDeviceMemory get_device_memory() { return memory; }
                glm::vec2 get_size() const { return dimensions; }
                bool is_sampler_set() const { return is_sampler; }
                bool is_depth_set() const { return is_depth; }
                
                void clean(const vk::device::handlers& dev);
            private:
                void create_sampler(const vk::device::handlers& dev);
                void allocate(const vk::device::handlers& dev);

                VkImage image;
                VkImageView image_view;
                VkDeviceMemory memory;

                VkSampler sampler;
                VkFormat format;
                glm::vec2 dimensions;
                VkDeviceSize device_size;

                // VkDescriptorSet ImGuiDescriptor;
                uint32_t ID = -1;
                bool is_sampler = false;
                bool is_depth = false;
                bool is_storage = false;
                bool is_cube = false;
                std::string name;
        };
   }
};

