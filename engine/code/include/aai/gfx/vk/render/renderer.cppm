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
import aai.gfx.vk.pipeline;
import aai.gfx.vk.model;
import std;
export {
    namespace vk {
        class renderer {
            public:
                void init(VkInstance inst, const vk::device::handlers& dev, VkDescriptorSet bindless);
                
                void block(VkCommandBuffer cmd, const rq::data& rq);
                void block_callback(VkCommandBuffer cmd, std::function<void(VkCommandBuffer cmd)> callback);

                rt::render_target* get_rt(rt::helper::val v)  { return rts.get_rt(v); }
                const rt::base::ref_map& get_rt_ref_map() const { return rts.get_rt_ref_map(); }
                rt::render_target* get_last_target() { return rts.get_rt(rt::helper::val::MAIN_COLOR); } 
                const rt::base::ref& get_attachment_ref(const rt::name::val r) { return rts.get_ref(r); }
                void clean_rts(const vk::device::handlers& dev) { rts.clean(dev); }
                void recreate_rts(const vk::device::handlers& dev) { rts.create(dev, window_size); }
                void set_window_size(glm::ivec2 w) { window_size = w; }

                void set_instance_bind(VkDeviceAddress buffer_addr) { instance_buffer_address = buffer_addr; }
                void set_inst_rect_bind(VkDeviceAddress buffer_addr) { inst_rect_buffer_address = buffer_addr; }
                void set_camera_bind(VkDeviceAddress buffer_addr) { camera_buffer_address = buffer_addr; }

                void refresh_state();

                template <typename T>
                void blocks(VkCommandBuffer cmd, rq::instanced<T>& rq)
                {
                    check_render_state(rq.material_handle);
                    start_render(cmd, rq.material_handle->attachment_ref);
                    if (rq.material_handle->dynamic_viewport) {
                        set_dynamic_viewport(cmd);
                    }
                    if (rq.mesh_handle) {
                            // todo hardcoded
                            VkDeviceSize offsets[2] = { 0, 0 };
                            VkBuffer vert_buf[2] = { rq.mesh_handle->get_vertex_buffer(), rq.buffer };
                            vkCmdBindVertexBuffers(cmd, 0, 2, vert_buf, offsets);
                            vkCmdBindIndexBuffer(cmd, rq.mesh_handle->get_index_buffer(), 0, VK_INDEX_TYPE_UINT16);
                            check_bindings(rq.material_handle, nullptr);
                            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, rq.material_handle->pipeline_layout , 0, 1, &bindless_set, 0, nullptr);
                            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, rq.material_handle->pipeline);

                            vk::pipeline::push_range constants;
                            constants.camera_gpu_address = camera_buffer_address;

                            vkCmdPushConstants(cmd, rq.material_handle->pipeline_layout , VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(vk::pipeline::push_range), &constants);

                            vkCmdDraw(cmd, 6, rq.data.size(), 0, 0);
                        }
                    end_render(cmd);
                }
                
            private:
                struct render_state {
                    rt::base::ref attachment_ref;
                    
                    bool bind_pipeline = true;
                    bool bind_mesh = true;
                    std::shared_ptr<mat::data> check_material = nullptr;
                    std::shared_ptr<model::base> check_mesh = nullptr;

                    uint32_t instance_ind = 0;
                };

                void check_render_state(std::shared_ptr<mat::data> material_handle);
                void check_bindings(std::shared_ptr<mat::data> material, std::shared_ptr<model::base> mesh);
                
                void set_dynamic_viewport(VkCommandBuffer cmd);
                void start_render(VkCommandBuffer cmd, const rt::base::ref& attachment_ref);
                void end_render(VkCommandBuffer cmd);
                void draw(VkCommandBuffer cmd, const rq::data& rq);
                void draw(VkCommandBuffer cmd, const rq::data& rq, const model::types::node& n);

                PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR{VK_NULL_HANDLE};
        	    PFN_vkCmdEndRenderingKHR   vkCmdEndRenderingKHR{VK_NULL_HANDLE};

                VkDescriptorSet bindless_set;
                VkDeviceAddress camera_buffer_address ;
                VkDeviceAddress instance_buffer_address ;
                VkDeviceAddress inst_rect_buffer_address ;

                rt::base rts;
                render_state state;

                glm::ivec2 window_size;
        };
    }
};

