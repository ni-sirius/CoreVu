#include "render_system.hpp"

// temp libs
#define GLM_FORCE_RADIANS           // to be sure that no change depending on system
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // instead of -1 to 1 ?
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
using namespace corevutest;

struct SimplePushConstantData // NOTE : ALL push data constants together are
                              // limited to 128 bytes space! But it's quite
                              // handy for storing transformation matrices.
{
  glm::mat2 transform{1.f};
  glm::vec2 offset; // 4 bytes * 2 = 8 bytes
  alignas(16) glm::vec3
      color; // w/o alignas will start with 9th byte // vulkan require
             // alignment of 4*size(val) for 3&4 component vectors. -> need to
             // add offset 16 bytes instead of (4bytes*2)of vec2 member offset.
};

RenderSystem::RenderSystem(
    corevu::CoreVuDevice& device, VkRenderPass render_pass)
  : m_corevu_device{device}
{
  createPipelineLayout();
  createPipeline(render_pass);
}

RenderSystem::~RenderSystem()
{
  vkDestroyPipelineLayout(m_corevu_device.device(), m_pipeline_layout, nullptr);
}

void RenderSystem::createPipelineLayout()
{
  VkPushConstantRange push_constant_range{};
  push_constant_range.stageFlags =
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  push_constant_range.offset =
      0; // if separate ranges for different stages are used.
  push_constant_range.size = sizeof(SimplePushConstantData);

  VkPipelineLayoutCreateInfo pipeline_layout_info{};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = 0;
  pipeline_layout_info.pSetLayouts = nullptr;
  pipeline_layout_info.pushConstantRangeCount =
      1; // 1 for one constant range for all stages
  pipeline_layout_info.pPushConstantRanges =
      &push_constant_range; // TODO what if 2 ranges?
  if (vkCreatePipelineLayout(
          m_corevu_device.device(), &pipeline_layout_info, nullptr,
          &m_pipeline_layout) != VK_SUCCESS)
  {
    throw std::runtime_error("FAILURE::can't create pipeline layout!");
  }
}

void RenderSystem::createPipeline(VkRenderPass render_pass)
{
  assert(
      m_pipeline_layout != nullptr &&
      "Cannot create pipeline before pipeline layout");

  corevu::PipelineConfigInfo pipeline_config{};
  corevu::CoreVuPipeline::DefaultPipelineConfigInfo(pipeline_config);
  pipeline_config.renderPass = render_pass;
  pipeline_config.pipelineLayout = m_pipeline_layout;
  m_corevu_pipeline = std::make_unique<corevu::CoreVuPipeline>(
      m_corevu_device, pipeline_config,
      "../corevu/shaders/simple_shader.vert.spv",
      "../corevu/shaders/simple_shader.frag.spv");
}

void RenderSystem::renderGameObjects(
    VkCommandBuffer command_buffer,
    std::vector<corevu::CoreVuGameObject>& game_objects)
{
  m_corevu_pipeline->Bind(command_buffer);

  for (auto& obj : game_objects)
  {
    obj.transform.rotation =
        glm::mod(obj.transform.rotation + 0.01f, glm::two_pi<float>());

    SimplePushConstantData push{};
    push.offset = obj.transform.translation;
    push.color = obj.color;
    push.transform = obj.transform.ToMat2();

    vkCmdPushConstants(
        command_buffer, m_pipeline_layout,
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0,
        sizeof(SimplePushConstantData), &push);

    obj.model->Bind(command_buffer);
    obj.model->Draw(command_buffer);
  }
}
