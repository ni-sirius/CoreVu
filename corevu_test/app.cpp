#include "app.hpp"

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

SampleApp::SampleApp()
{
  loadGameObjects();
  createPipelineLayout();
  createPipeline();
}

SampleApp::~SampleApp()
{
  vkDestroyPipelineLayout(m_corevu_device.device(), m_pipeline_layout, nullptr);
}

void SampleApp::run()
{
  while (!m_corevu_window.shouldClose())
  {
    glfwPollEvents(); // on some pltforms processing of events can block
                      // polling. The window refresh callback can be used to
                      // fix that.

    if (auto command_buffer = m_renderer.BeginFrame())
    {
      m_renderer.BeginSwapChainRenderPass(command_buffer);
      renderGameObjects(command_buffer);
      m_renderer.EndSwapChainRenderPass(command_buffer);
      m_renderer.EndFrame();
    }
  }

  vkDeviceWaitIdle(m_corevu_device.device());
}

void SampleApp::createPipelineLayout()
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

void SampleApp::createPipeline()
{
  assert(
      m_pipeline_layout != nullptr &&
      "Cannot create pipeline before pipeline layout");

  corevu::PipelineConfigInfo pipeline_config{};
  corevu::CoreVuPipeline::DefaultPipelineConfigInfo(pipeline_config);
  pipeline_config.renderPass = m_renderer.GetSwapchainRenderpass();
  pipeline_config.pipelineLayout = m_pipeline_layout;
  m_corevu_pipeline = std::make_unique<corevu::CoreVuPipeline>(
      m_corevu_device, pipeline_config,
      "../corevu/shaders/simple_shader.vert.spv",
      "../corevu/shaders/simple_shader.frag.spv");
}

// static void sierpinski(
//     std::vector<corevu::CoreVuModel::Vertex>& vertices, int depth,
//     glm::vec2 left, glm::vec2 right, glm::vec2 top)
// {
//   if (depth <= 0)
//   {
//     vertices.push_back({top});
//     vertices.push_back({right});
//     vertices.push_back({left});
//   }
//   else
//   {
//     auto leftTop = 0.5f * (left + top);
//     auto rightTop = 0.5f * (right + top);
//     auto leftRight = 0.5f * (left + right);
//     sierpinski(vertices, depth - 1, left, leftRight, leftTop);
//     sierpinski(vertices, depth - 1, leftRight, right, rightTop);
//     sierpinski(vertices, depth - 1, leftTop, rightTop, top);
//   }
// }

void SampleApp::loadGameObjects()
{
  // base solution
  std::vector<corevu::CoreVuModel::Vertex> vertices{
      {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
      {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

  // sierpinski solution
  // std::vector<corevu::CoreVuModel::Vertex> vertices{};
  // sierpinski(vertices, 5, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.0f, -0.5f});

  auto corevu_model =
      std::make_shared<corevu::CoreVuModel>(m_corevu_device, vertices);

  auto triangle = corevu::CoreVuGameObject::Create();
  triangle.model = corevu_model;
  triangle.color = {.1f, .8f, .1f};
  triangle.transform.translation.x = .2f;
  triangle.transform.scale = {2.f, .5f};
  triangle.transform.rotation = .25f * glm::two_pi<float>();

  m_game_objects.emplace_back(std::move(triangle));
}

void SampleApp::renderGameObjects(VkCommandBuffer command_buffer)
{
  m_corevu_pipeline->Bind(command_buffer);

  for (auto& obj : m_game_objects)
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
