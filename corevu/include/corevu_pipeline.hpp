#pragma once

#include <string>
#include <vector>

#include <corevu_device.hpp>

namespace corevu
{
struct PipelineConfigInfo {
  PipelineConfigInfo(const PipelineConfigInfo&) = delete;
  PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;
  // Explicit default constructor
  PipelineConfigInfo() 
    : viewportInfo{}, 
      inputAssemblyInfo{}, 
      rasterizationInfo{}, 
      multisampleInfo{}, 
      colorBlendAttachment{}, 
      colorBlendInfo{}, 
      depthStencilInfo{}, 
      dynamicStateEnables{}, 
      dynamicStateInfo{},
      pipelineLayout(nullptr),
      renderPass(nullptr),
      subpass(0) {}

  VkPipelineViewportStateCreateInfo viewportInfo;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  VkPipelineRasterizationStateCreateInfo rasterizationInfo;
  VkPipelineMultisampleStateCreateInfo multisampleInfo;
  VkPipelineColorBlendAttachmentState colorBlendAttachment;
  VkPipelineColorBlendStateCreateInfo colorBlendInfo;
  VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
  std::vector<VkDynamicState> dynamicStateEnables;
  VkPipelineDynamicStateCreateInfo dynamicStateInfo;
  VkPipelineLayout pipelineLayout = nullptr;
  VkRenderPass renderPass = nullptr;
  uint32_t subpass = 0;
};

class CoreVuPipeline
{
public:
  CoreVuPipeline(
      CoreVuDevice& device, const PipelineConfigInfo& config_info,
      const std::string& vert_filepath, const std::string& frag_filepath);
  ~CoreVuPipeline();
  CoreVuPipeline(const CoreVuPipeline&) = delete;
  CoreVuPipeline& operator=(const CoreVuPipeline&) = delete;

  void Bind(VkCommandBuffer command_buffer);

  static void DefaultPipelineConfigInfo(PipelineConfigInfo& config_info);

private:
  static std::vector<char> readFile(const std::string& filepath);
  void createGraphicsPipeline(
      const std::string& vert_filepath, const std::string& frag_filepath,
      const PipelineConfigInfo& config_info);

  void createShaderModule(
      const std::vector<char>& code, VkShaderModule* shader_module);

  CoreVuDevice& m_device;
  VkPipeline m_graphics_pipeline;
  VkShaderModule m_vulkan_vert_shader_module;
  VkShaderModule m_vulkan_frag_shader_module;
};
} // namespace corevu