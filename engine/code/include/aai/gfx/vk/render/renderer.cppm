module;
#include "aai/gfx/vk/backend/vk_defines.h"
#include "aai/gfx/vk/model/model_types.h"
#include <vulkan/vulkan_core.h>

export module aai.gfx.vk.renderer;
export import aai.gfx.vk.rt;
export import aai.gfx.vk.device;
export import aai.gfx.vk.rt.helper;
export import aai.gfx.vk.rq;
export import aai.gfx.materials;
import aai.gfx.vk.model;
import std;
export {
    namespace vk {
        class renderer {
            public:
                void init(VkInstance inst, const vk::device::handlers& dev, VkDescriptorSet bindless, VkDeviceAddress buffer_addr);
                
                void block(VkCommandBuffer cmd, const rq::data& rq);

                rt::render_target* get_rt(rt::helper::val v)  { return rts.get_rt(v); }
                const rt::base::ref_map& get_rt_ref_map() const { return rts.get_rt_ref_map(); }
                rt::render_target* get_last_target() { return rts.get_rt(rt::helper::val::MAIN_COLOR); } 
                const rt::base::ref& get_attachment_ref(const rt::base::name r) { return rts.get_ref(r); }
                void clean_rts(const vk::device::handlers& dev) { rts.clean(dev); }
                void recreate_rts(const vk::device::handlers& dev) { rts.create(dev, window_size); }
                void set_window_size(glm::ivec2 w) { window_size = w; }
            private:
                struct render_state {
                    rt::base::ref attachment_ref;
                    rt::helper::rule attachment_rule;

                    bool bind_pipeline = true;
                    bool bind_mesh = true;
                    mat::data* check_material = nullptr;
                    std::shared_ptr<model::base> check_mesh = nullptr;
                };

                void check_render_state(const rq::data& rq);
                void check_bindings(mat::data* material, std::shared_ptr<model::base> mesh);
                
                void set_dynamic_viewport(VkCommandBuffer cmd);
                void start_render(VkCommandBuffer cmd, const rt::base::ref& attachment_ref);
                void end_render(VkCommandBuffer cmd);
                void draw(VkCommandBuffer cmd, const rq::data& rq);
                void draw(VkCommandBuffer cmd, const rq::data& rq, const model::types::node* n);

                PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR{VK_NULL_HANDLE};
        	    PFN_vkCmdEndRenderingKHR   vkCmdEndRenderingKHR{VK_NULL_HANDLE};

                VkDescriptorSet bindless_set;
                VkDeviceAddress buffer_address;

                rt::base rts;
                render_state state;

                glm::ivec2 window_size;
        };
    }
};

