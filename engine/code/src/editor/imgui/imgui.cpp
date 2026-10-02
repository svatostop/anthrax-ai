module;
#include <stdlib.h>
#include <stdio.h>
#include <vulkan/vulkan_core.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

module aai.editor.imgui;

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
    init_info.PipelineCache = pipeline_cache; 
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
