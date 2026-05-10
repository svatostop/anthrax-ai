module;
#include "aai/gfx/vk/backend/vk_defines.h"
#include <vulkan/vulkan_core.h>

export module aai.gfx.vk.renderer;
export import aai.gfx.vk.rt;
export import aai.gfx.vk.device;
export import aai.gfx.vk.rt.helper;
export import aai.gfx.vk.rq;

export {
    namespace vk {
        class renderer {
            public:
                void init(VkInstance inst, const vk::device::handlers& dev, VkDescriptorSet bindless);
                
                void block(VkCommandBuffer cmd, const rq::data& rq);

                rt::render_target* get_rt(rt::helper::val v)  { return rts.get_rt(v); }
                rt::render_target* get_last_target() { return rts.get_rt(rt::helper::val::MAIN_COLOR); } 
                const rt::base::ref& get_attachment_ref(const rt::base::name r) { return rts.get_ref(r); }
            private:
                void start_render(VkCommandBuffer cmd, const rt::base::ref& attachment_ref);
                void end_render(VkCommandBuffer cmd);
                void draw(VkCommandBuffer cmd, const rq::data& rq);

                PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR{VK_NULL_HANDLE};
        	    PFN_vkCmdEndRenderingKHR   vkCmdEndRenderingKHR{VK_NULL_HANDLE};

                VkDescriptorSet bindless_set;

                rt::base rts; 
        };
    }
};

