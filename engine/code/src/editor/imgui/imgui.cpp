module;
#include <cstdint>
#include <glm/geometric.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vulkan/vulkan_core.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "aai/gfx/vk/model/model_types.h"

module aai.editor.imgui;
import aai.gfx.debug;
import aai.gfx.font;
import aai.utils.threads;
import aai.gfx.vk.model.anim;
import aai.io;

void imgui_backend::shutdown(VkDevice device) 
{
    if (!is_init)
        return;
    vkDestroyDescriptorPool(device, pool, nullptr);
    ImGui_ImplVulkan_Shutdown();
}

void imgui_backend::new_frame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}
void imgui_backend::render()
{
    ImGui::Render();
}
void imgui_backend::show_demo_window()
{
    ImGui::ShowDemoWindow();
}

void imgui_backend::backend_render_data(VkCommandBuffer cmd)
{
    if (!ImGui::GetDrawData())
        return;
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
}

template <typename T>
void combo_lookup(T& current_value, const std::string& label)
{
    int idx = 0;
    if (ImGui::BeginCombo(label.c_str(), get_value(current_value).c_str()))
    {
        for (int n = 0; n < T::SIZE; n++)
            if (ImGui::Selectable(get_value(static_cast<T>(n)).c_str(), idx == n)) {
                idx = n;
                current_value = static_cast<T>(n);
            }
        ImGui::EndCombo();
    }
}

void imgui_backend::process()
{
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
    ImVec2 win_pos = ImGui::GetWindowPos();
    ImVec2 win_size = ImGui::GetWindowSize();

    aai::io::mouse::set_in_editor(false);
    if (aai::io::mouse::get_pos().x > win_pos.x && aai::io::mouse::get_pos().x < win_pos.x + win_size.x  &&
    aai::io::mouse::get_pos().y > win_pos.y && aai::io::mouse::get_pos().y < win_pos.y + win_size.y)
       aai::io::mouse::set_in_editor(true);

    ImGui::Begin("new engine");
    if (ImGui::BeginTabBar("MyTabBar")) {
        if (ImGui::BeginTabItem("animations"))
        {
            anim::base* anim = reinterpret_cast<anim::base*>(output_datas[utils::editor_types::ANIMATION]);
            if (anim && anim->is_model_valid()) {
                int i = 0;
                const std::vector<std::string>& names = anim->get_clip_names();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 1.0, 0.0, 0.9));
                ImGui::SeparatorText("Clips"); 
                ImGui::PopStyleColor();
                ImGui::Separator();
                
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 1.0, 0.0, 0.9));
                ImGui::Text("weight blend method");
                ImGui::PopStyleColor();
                ImGui::Separator();
                
                combo_lookup<anim::blend_method::val>(anim->blend_method, "blend method");

                uint32_t blend_factor_size = anim->get_blend_factors().size();
                if (anim->blend_method == anim::blend_method::val::FACTOR) {
                    if (ImGui::Button("add blend factor")) {
                        anim->add_blend_factor();
                    }
                    int j = 0;
                    for (float& f : anim->get_blend_factors()) {
                        std::string text_label = "blend factor [" + std::to_string(j) + "]";
                        ImGui::DragFloat(text_label.c_str(), &f, 0.001, 0.0f, 1.0f, "%.3f");
                        j++;
                    }
                }

                ImGui::Separator();
                
                if (ImGui::Button("add clip")) {
                    auto& clip2 = anim->add_clip();
                    clip2.clip = 1;
                    clip2.loop = true;
                    clip2.weight_to_sum = 0.0f;
                    anim->play(anim->clips.size() - 1);
                }

                for (anim::base::clip_info& clip : anim->clips) {
                    int idx = clip.clip;
                    std::string text_label = "clip info for [" + std::to_string(i) + "]";
                    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1, 0.1, 0.1, 1));
                    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.07, 0.07, 0.07, 1));
                    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.07, 0.07, 0.07, 1));
                    if (ImGui::TreeNodeEx(text_label.c_str(), ImGuiTreeNodeFlags_Framed)) {
                        ImGui::PopStyleColor();
                        ImGui::PopStyleColor();
                        ImGui::PopStyleColor();
                        ImGui::PushID(i);
  
                        int32_t new_clip = clip.clip;
                        if (ImGui::BeginCombo("clips", names[clip.clip].c_str()))
                        {
                            for (int n = 0; n < names.size(); n++)
                                if (ImGui::Selectable(names[n].c_str(), idx == n)) {
                                    idx = n;
                                    new_clip = n; 
                                }
                            ImGui::EndCombo();
                        }ImGui::SameLine();
                        if (ImGui::Button("remove clip")) {
                            anim->remove_clip(i);
                            ImGui::PopID();
                            ImGui::TreePop();
                            break;
                        }
                        ImGui::Checkbox("loop", &clip.loop);
                        ImGui::Separator();
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 1.0, 0.0, 0.9));
                        ImGui::Text("%s", "mask:"); 
                        ImGui::PopStyleColor();
                        combo_lookup<anim::mask::val>(clip.mask_type, "type");
                        ImGui::Separator();
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 1.0, 0.0, 0.9));
                        ImGui::Text("%s", "weights:"); 
                        ImGui::PopStyleColor();
                        if (anim->blend_method == anim::blend_method::val::WEIGHT_SUM)
                            ImGui::DragFloat("weight to sum", &clip.weight_to_sum, 0.01, 0.0f, 1.0f, "%.2f");

                        if (anim->blend_method == anim::blend_method::val::FACTOR) {

                            ImGui::Columns(2);
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 1.0, 0.0, 0.9));
                            ImGui::Text("%s", "first multiplier:"); 
                            ImGui::PopStyleColor();
                            ImGui::PushID(100);
                            std::string id_str = std::to_string(clip.mults[0].blend_factor_ind);
                            int blend_ind = clip.mults[0].blend_factor_ind;
                            if (ImGui::BeginCombo("blind ids", id_str.c_str()))
                            {
                                 for (int n = 0; n < anim->get_blend_factors().size(); n++)
                                    if (ImGui::Selectable(std::to_string(n).c_str(), blend_ind == n)) {
                                        blend_ind = n;
                                        clip.mults[0].blend_factor_ind = n; 
                                    }
                                ImGui::EndCombo();
                            }
                            ImGui::Checkbox("is reversed", &clip.mults[0].is_reversed);ImGui::SameLine();
                            ImGui::Checkbox("is blank", &clip.mults[0].is_blank);
                            ImGui::PopID();
                            ImGui::NextColumn();
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 1.0, 0.0, 0.9));
                            ImGui::Text("%s", "second multiplier:"); 
                            ImGui::PopStyleColor();
                            ImGui::PushID(101);
                            std::string id_str2 = std::to_string(clip.mults[1].blend_factor_ind);
                            int blend_ind2 = clip.mults[1].blend_factor_ind;
                            if (ImGui::BeginCombo("blind ids", id_str2.c_str()))
                            {
                                 for (int n = 0; n < anim->get_blend_factors().size(); n++)
                                    if (ImGui::Selectable(std::to_string(n).c_str(), blend_ind2 == n)) {
                                        blend_ind2 = n;
                                        clip.mults[1].blend_factor_ind = n; 
                                    }
                                ImGui::EndCombo();
                            }

                            ImGui::Checkbox("is reversed", &clip.mults[1].is_reversed);ImGui::SameLine();
                            ImGui::Checkbox("is blank", &clip.mults[1].is_blank);
                            ImGui::PopID();
                            ImGui::Columns(1);
                        }
                        ImGui::Separator();
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 1.0, 0.0, 0.9));
                        ImGui::DragFloat("offset time", &clip.offset_time, 0.1, 0.0f, 1.0f, "%.1f");
                        ImGui::PopStyleColor();
                        if (new_clip != clip.clip) {
                            clip.clip = new_clip;
                            anim->play(i);
                        }
                        ImGui::PopID();
                        ImGui::TreePop();
                    }
                    else {
                        ImGui::PopStyleColor();
                        ImGui::PopStyleColor();
                        ImGui::PopStyleColor();
                    }
                    i++;
                    ImGui::Separator();
                } 
            }
            
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 1.0, 0.0, 0.9));
            ImGui::SeparatorText("Other"); 
            ImGui::PopStyleColor();
            ImGui::Separator();
            bool enable = gfx::dbg::draw::get()->enabled();
            ImGui::Checkbox("enable debug draw", &enable);
            gfx::dbg::draw::get()->set_enabled(enable);

            float anim_mult = gfx::dbg::draw::get()->get_global_anim_mult();
            ImGui::DragFloat("global anim time mult", &anim_mult, 0.1, 0.0f, 2.0f, "%.1f");
            gfx::dbg::draw::get()->set_global_anim_mult(anim_mult);

           
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("threads")) {
            std::map<std::string, utils::thread_dbg::info> stats = utils::thread_dbg::get()->take();
            {
                for (auto& it : stats) {
                    ImGui::Columns(5);
                    ImGui::Text("%s", it.first.c_str());
                    ImGui::NextColumn();
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
                    ImGui::Text("state: ");
                    ImGui::PopStyleColor();
                    ImGui::NextColumn();
                    if (it.second.st == utils::thread_state::val::RUNNING) {
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
                    }
                    ImGui::Text("%s", utils::thread_state::get_value(it.second.st).c_str());
                    if (it.second.st == utils::thread_state::val::RUNNING) {
                        ImGui::PopStyleColor();
                    }
                    ImGui::NextColumn();
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
                    ImGui::Text("time: ");
                    ImGui::PopStyleColor();
                    ImGui::NextColumn();
                    ImGui::Text("%.2fms", it.second.ms);
                    ImGui::NextColumn();
                    ImGui::Separator();
                }
            }
            ImGui::Columns(1);
            ImGui::EndTabItem(); 
        }
        if (ImGui::BeginTabItem("ui"))
        {
            ui::font::context* ft = reinterpret_cast<ui::font::context*>(output_datas[utils::editor_types::FONT]);
                
            if (ft) {
                ImGui::Text("font size: %d", ft->size); ImGui::SameLine();
                static char input_buf[1024];
                ImGui::InputText("font text", input_buf, 1024);
                ft->text = input_buf;
                ImGui::DragFloat("scale", &ft->scale, 0.1, 0.0f, 2.0f, "%.1f");
                float pos[2] = { ft->pos.x, ft->pos.y };
                ImGui::DragFloat2("position", pos, 1.0, 0.0f, 2000.0f, "%.1f");
                ft->pos = glm::vec2(pos[0], pos[1]);
                ImGui::DragFloat("size x", &ft->size_x, 1.0, 1.0f, 2000.0f, "%.1f");
                combo_lookup<ui::alignment::val>(ft->align, "alignment");
            }
            ImGui::EndTabItem(); 
        }
        ImGui::EndTabBar();
    }
    
    ImGui::End();
}

static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

void imgui_backend::init(GLFWwindow* window, VkInstance instance, VkPhysicalDevice physical_device, VkDevice device, VkQueue queue, std::uint32_t queue_family)
{
    VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, IMGUI_IMPL_VULKAN_MINIMUM_SAMPLED_IMAGE_POOL_SIZE },
        { VK_DESCRIPTOR_TYPE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_SAMPLER_POOL_SIZE },
    };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 0;
    for (VkDescriptorPoolSize& pool_size : pool_sizes)
        pool_info.maxSets += pool_size.descriptorCount;
    pool_info.poolSizeCount = (uint32_t)IM_COUNTOF(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    vkCreateDescriptorPool(device, &pool_info, nullptr, &pool);
    
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();

    VkPipelineRenderingCreateInfoKHR pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR; 
    pipeline_info.colorAttachmentCount = 1;
    VkFormat format = VK_FORMAT_R16G16B16A16_SFLOAT;
    pipeline_info.pColorAttachmentFormats = &format;
    pipeline_info.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;

    ImGui_ImplGlfw_InitForVulkan(window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = instance;
	init_info.PhysicalDevice = physical_device;
	init_info.Device = device;
	init_info.Queue = queue;
	init_info.QueueFamily = queue_family;
	init_info.DescriptorPool = pool;
    init_info.PipelineCache = VK_NULL_HANDLE; 
	init_info.MinImageCount = 2;
	init_info.ImageCount = 3;
    init_info.UseDynamicRendering = true;
    init_info.PipelineInfoMain.RenderPass = nullptr;
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo = pipeline_info;
    init_info.CheckVkResultFn = check_vk_result;
	ImGui_ImplVulkan_Init(&init_info);

    io.Fonts->AddFontDefaultVector();

    // submit_callback(device, queue, [=](VkCommandBuffer cmd) {
    //     ImGui_ImplVulkan_CreateFontsTexture(cmd);
    // });
    // ImGui_ImplVulkan_DestroyFontUploadObjects();

    is_init = true;
}
