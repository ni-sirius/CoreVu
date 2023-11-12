#include <corevu_pipeline.hpp>

#include <fstream>
#include <stdexcept>

#include <iostream>

#include <cassert>

using namespace corevu;

CoreVuPipeline::CoreVuPipeline(
    CoreVuDevice& device, const PipelineConfigInfo& config_info,
    const std::string& vert_filepath, const std::string& frag_filepath)
  : m_device{device}
{
  createGraphicsPipeline(vert_filepath, frag_filepath, config_info);
}

CoreVuPipeline::~CoreVuPipeline()
{
  vkDestroyShaderModule(
      m_device.device(), m_vulkan_vert_shader_module, nullptr);
  vkDestroyShaderModule(
      m_device.device(), m_vulkan_frag_shader_module, nullptr);
  vkDestroyPipeline(m_device.device(), m_vulkan_pipeline, nullptr);
}

PipelineConfigInfo CoreVuPipeline::DefaultPipelineConfigInfo(
    uint32_t width, uint32_t height)
{
  PipelineConfigInfo config_info{};

  config_info.inputAssemblyInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  config_info.inputAssemblyInfo.topology =
      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; //defines a way vertices are interpreted
  config_info.inputAssemblyInfo.primitiveRestartEnable =
      VK_FALSE; // if a brake in triagle fan is needed

  config_info.viewport.x = 0.0f;
  config_info.viewport.y = 0.0f;
  config_info.viewport.width = static_cast<float>(width);
  config_info.viewport.height = static_cast<float>(height);
  config_info.viewport.minDepth = 0.0f;
  config_info.viewport.maxDepth = 1.0f;

  config_info.scissor.offset = {0, 0};
  config_info.scissor.extent = {width, height};

  config_info.viewportInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  config_info.viewportInfo.viewportCount = 1;
  config_info.viewportInfo.pViewports = &config_info.viewport;
  config_info.viewportInfo.scissorCount = 1;
  config_info.viewportInfo.pScissors = &config_info.scissor;

  config_info.rasterizationInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  config_info.rasterizationInfo.depthClampEnable = VK_FALSE;
  config_info.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
  config_info.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
  config_info.rasterizationInfo.lineWidth = 1.0f;
  config_info.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
  config_info.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
  config_info.rasterizationInfo.depthBiasEnable = VK_FALSE;
  config_info.rasterizationInfo.depthBiasConstantFactor = 0.0f; // Optional
  config_info.rasterizationInfo.depthBiasClamp = 0.0f;          // Optional
  config_info.rasterizationInfo.depthBiasSlopeFactor = 0.0f;    // Optional

  config_info.multisampleInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  config_info.multisampleInfo.sampleShadingEnable = VK_FALSE;
  config_info.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  config_info.multisampleInfo.minSampleShading = 1.0f;          // Optional
  config_info.multisampleInfo.pSampleMask = nullptr;            // Optional
  config_info.multisampleInfo.alphaToCoverageEnable = VK_FALSE; // Optional
  config_info.multisampleInfo.alphaToOneEnable = VK_FALSE;      // Optional

  config_info.colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  config_info.colorBlendAttachment.blendEnable = VK_FALSE;
  config_info.colorBlendAttachment.srcColorBlendFactor =
      VK_BLEND_FACTOR_ONE; // Optional
  config_info.colorBlendAttachment.dstColorBlendFactor =
      VK_BLEND_FACTOR_ZERO;                                        // Optional
  config_info.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
  config_info.colorBlendAttachment.srcAlphaBlendFactor =
      VK_BLEND_FACTOR_ONE; // Optional
  config_info.colorBlendAttachment.dstAlphaBlendFactor =
      VK_BLEND_FACTOR_ZERO;                                        // Optional
  config_info.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

  config_info.colorBlendInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  config_info.colorBlendInfo.logicOpEnable = VK_FALSE;
  config_info.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
  config_info.colorBlendInfo.attachmentCount = 1;
  config_info.colorBlendInfo.pAttachments = &config_info.colorBlendAttachment;
  config_info.colorBlendInfo.blendConstants[0] = 0.0f; // Optional
  config_info.colorBlendInfo.blendConstants[1] = 0.0f; // Optional
  config_info.colorBlendInfo.blendConstants[2] = 0.0f; // Optional
  config_info.colorBlendInfo.blendConstants[3] = 0.0f; // Optional

  config_info.depthStencilInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  config_info.depthStencilInfo.depthTestEnable = VK_TRUE;
  config_info.depthStencilInfo.depthWriteEnable = VK_TRUE;
  config_info.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
  config_info.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
  config_info.depthStencilInfo.minDepthBounds = 0.0f; // Optional
  config_info.depthStencilInfo.maxDepthBounds = 1.0f; // Optional
  config_info.depthStencilInfo.stencilTestEnable = VK_FALSE;
  config_info.depthStencilInfo.front = {}; // Optional
  config_info.depthStencilInfo.back = {};  // Optional

  return config_info;
}

std::vector<char> CoreVuPipeline::readFile(const std::string& filepath)
{
  std::ifstream file{
      filepath,
      std::ios::ate | std::ios::binary}; // go to the end|read as binary
  if (!file.is_open())
  {
    throw std::runtime_error("FAILURE::can't open file:" + filepath);
  }

  size_t file_size = static_cast<size_t>(file.tellg());
  std::vector<char> buffer(file_size);

  file.seekg(0); // go to start of file
  file.read(buffer.data(), file_size);

  file.close();

  return buffer;
}

void CoreVuPipeline::createGraphicsPipeline(
    const std::string& vert_filepath, const std::string& frag_filepath,
    const PipelineConfigInfo& config_info)
{
  assert(
      config_info.pipelineLayout != VK_NULL_HANDLE &&
      "ASSERT::can't create pipeline::no pipeline layout pirovided in config "
      "info.");
  assert(
      config_info.renderPass != VK_NULL_HANDLE &&
      "ASSERT::can't create pipeline::no render pass pirovided in config "
      "info.");

  auto vert_code = readFile(vert_filepath);
  auto frag_code = readFile(frag_filepath);

  createShaderModule(vert_code, &m_vulkan_vert_shader_module);
  createShaderModule(frag_code, &m_vulkan_frag_shader_module);

  VkPipelineShaderStageCreateInfo shader_stages[2];
  shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  shader_stages[0].module = m_vulkan_vert_shader_module;
  shader_stages[0].pName = "main";
  shader_stages[0].flags = 0;
  shader_stages[0].pNext = nullptr;
  shader_stages[0].pSpecializationInfo = nullptr;

  shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  shader_stages[1].module = m_vulkan_frag_shader_module;
  shader_stages[1].pName = "main";
  shader_stages[1].flags = 0;
  shader_stages[1].pNext = nullptr;
  shader_stages[1].pSpecializationInfo = nullptr;

  VkPipelineVertexInputStateCreateInfo vertex_input_info;
  vertex_input_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_info.vertexAttributeDescriptionCount = 0;
  vertex_input_info.vertexBindingDescriptionCount = 0;
  vertex_input_info.pVertexAttributeDescriptions = nullptr;
  vertex_input_info.pVertexBindingDescriptions = nullptr;

  VkGraphicsPipelineCreateInfo pipeline_info;
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount = 2;
  pipeline_info.pStages = shader_stages;
  pipeline_info.pVertexInputState = &vertex_input_info;
  pipeline_info.pInputAssemblyState = &config_info.inputAssemblyInfo;
  pipeline_info.pViewportState = &config_info.viewportInfo;
  pipeline_info.pRasterizationState = &config_info.rasterizationInfo;
  pipeline_info.pMultisampleState = &config_info.multisampleInfo;
  pipeline_info.pColorBlendState = &config_info.colorBlendInfo;
  pipeline_info.pDepthStencilState = &config_info.depthStencilInfo;
  pipeline_info.pDynamicState = nullptr;

  pipeline_info.layout = config_info.pipelineLayout;
  pipeline_info.renderPass = config_info.renderPass;
  pipeline_info.subpass = config_info.subpass;

  pipeline_info.basePipelineIndex = -1;
  pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

  if (vkCreateGraphicsPipelines(
          m_device.device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr,
          &m_vulkan_pipeline) != VK_SUCCESS)
  {
    throw std::runtime_error("FAILURE::can't create graphics pipeline");
  }

  std::cout << "file is read for " << vert_filepath << " "
            << (int)vert_code.size() << std::endl;
  std::cout << "file is read for " << frag_filepath << " "
            << (int)frag_code.size() << std::endl;
}

void CoreVuPipeline::createShaderModule(
    const std::vector<char>& code, VkShaderModule* shader_module)
{
  VkShaderModuleCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = code.size();
  create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

  if (vkCreateShaderModule(
          m_device.device(), &create_info, nullptr, shader_module) !=
      VK_SUCCESS)
  {
    throw std::runtime_error("FAILURE::can't create shader module");
  }
}
