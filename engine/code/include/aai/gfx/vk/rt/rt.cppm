module;
#include "aai/gfx/vk/backend/vk_defines.h"

export module aai.gfx.vk.rt;
import aai.gfx.vk.device;
import aai.gfx.vk.rt.helper;
import aai.utils;
import std;
import glm;
export {
   namespace rt {
       class render_target {
           public:
                render_target() { name = "please dont call this ctor"; }
                render_target(const std::string& n) : name(n) {}
                render_target(uint32_t ind) { name = "test_name"; id = ind; }
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
                void set_name(const std::string& n) { name = n; }

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
                uint32_t id = -1;
                bool is_sampler = false;
                bool is_depth = false;
                bool is_storage = false;
                bool is_cube = false;
                std::string name;
       };

       class base {
           public:
               struct type {
                   rt::helper::val v;
                   VkFormat format;
               };
               enum class name {
                   ONE_QUAD = 0,
               };
               struct ref {
                   uint32_t color_count = 0;
                   uint32_t depth_count = 0;
                   std::vector<type> color_types;
                   type depth_types;
                   uint32_t id = 0;
               };
               typedef std::map<name, ref> ref_map;
               void fill_refs() {
                   ref r;
                   r.color_types.push_back({ rt::helper::val::MAIN_COLOR, VK_FORMAT_R8G8B8A8_UNORM });
                   r.color_count = r.color_types.size();
                   r.id = ++refs_counter;
                   refs[name::ONE_QUAD] = r;
               }           
               const ref& get_ref(name d)  { return refs[d]; }
               const ref_map& get_rt_ref_map() const { return refs; }

               void clean(const vk::device::handlers& dev) { 
                   for (int i = 0; i < static_cast<int>(rt::helper::val::SIZE); i++) {
                       if (rts[i]) {
                           rts[i]->clean(dev);
                           delete rts[i];
                       }
                   }
               }
               void create(const vk::device::handlers& dev, glm::ivec2 window_size) {
                   rts[static_cast<int>(rt::helper::val::MAIN_COLOR)] = new render_target(static_cast<int>(rt::helper::val::MAIN_COLOR));
                   rts[static_cast<int>(rt::helper::val::MAIN_COLOR)]->set_format(VK_FORMAT_R8G8B8A8_UNORM);
                   rts[static_cast<int>(rt::helper::val::MAIN_COLOR)]->set_dimensions({window_size.x, window_size.y});
                   rts[static_cast<int>(rt::helper::val::MAIN_COLOR)]->set_sampler(true);
                   rts[static_cast<int>(rt::helper::val::MAIN_COLOR)]->create(dev);
               }
               rt::render_target* get_rt(rt::helper::val v)  { return rts[static_cast<int>(v)]; }
           private:
               ref_map refs;
               uint32_t refs_counter = 0;
               rt::render_target* rts[static_cast<int>(rt::helper::val::SIZE)];
       };
   }
};

