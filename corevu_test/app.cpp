#include "app.hpp"

// std
#include <array>
using namespace corevutest;

TestApp::TestApp()
{
  loadModels();
  createPipelineLayout();
  createPipeline();
  createCommandBuffers();
}

TestApp::~TestApp()
{
  vkDestroyPipelineLayout(m_corevu_device.device(), m_pipeline_layout, nullptr);
}

void TestApp::createPipelineLayout()
{
  VkPipelineLayoutCreateInfo pipeline_layout_info{};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = 0;
  pipeline_layout_info.pSetLayouts = nullptr;
  pipeline_layout_info.pushConstantRangeCount = 0;
  pipeline_layout_info.pPushConstantRanges = nullptr;
  if (vkCreatePipelineLayout(
          m_corevu_device.device(), &pipeline_layout_info, nullptr,
          &m_pipeline_layout) != VK_SUCCESS)
  {
    throw std::runtime_error("FAILURE::can't create pipeline layout!");
  }
}

void TestApp::createPipeline()
{
  auto pipeline_config = corevu::CoreVuPipeline::DefaultPipelineConfigInfo(
      m_corevu_swapchain.width(), m_corevu_swapchain.height());
  pipeline_config.renderPass = m_corevu_swapchain.getRenderPass();
  pipeline_config.pipelineLayout = m_pipeline_layout;
  m_corevu_pipeline = std::make_unique<corevu::CoreVuPipeline>(
      m_corevu_device, pipeline_config,
      "../corevu/shaders/simple_shader.vert.spv",
      "../corevu/shaders/simple_shader.frag.spv");
}

void TestApp::createCommandBuffers()
{
  m_command_buffers.resize(m_corevu_swapchain.imageCount());

  VkCommandBufferAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandPool = m_corevu_device.getCommandPool();
  alloc_info.commandBufferCount =
      static_cast<uint32_t>(m_command_buffers.size());

  if (vkAllocateCommandBuffers(
          m_corevu_device.device(), &alloc_info, m_command_buffers.data()) !=
      VK_SUCCESS)
  {
    throw std::runtime_error("FAILURE::can't create command buffers!");
  }

  for (size_t i = 0; i < m_command_buffers.size(); i++)
  {
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(m_command_buffers[i], &begin_info) != VK_SUCCESS)
    {
      throw std::runtime_error("FAILURE::can't begin recording frame buffer!");
    }

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = m_corevu_swapchain.getRenderPass();
    render_pass_info.framebuffer = m_corevu_swapchain.getFrameBuffer(i);

    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent =
        m_corevu_swapchain.getSwapChainExtent();

    // it's specified in corevu_swap_chain that first attachement is color and
    // second is depth
    std::array<VkClearValue, 2> clear_values{};
    clear_values[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
    clear_values[1].depthStencil = {1.0f, 0};
    render_pass_info.clearValueCount =
        static_cast<uint32_t>(clear_values.size());
    render_pass_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(
        m_command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    m_corevu_pipeline->Bind(m_command_buffers[i]);
    // vkCmdDraw(m_command_buffers[i], 3, 1, 0, 0); // for hardcoded
    // implementation
    m_corevu_model->Bind(m_command_buffers[i]);
    m_corevu_model->Draw(m_command_buffers[i]);

    vkCmdEndRenderPass(m_command_buffers[i]);
    if (vkEndCommandBuffer(m_command_buffers[i]))
    {
      std::runtime_error("FAILURE::can't end recording of frame buffer!");
    }
  }
}

void TestApp::drawFrame()
{
  uint32_t image_index;
  auto result = m_corevu_swapchain.acquireNextImage(&image_index);

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
  {
    throw std::runtime_error("FAILURE:: can't acuire swap chain image!");
  }

  result = m_corevu_swapchain.submitCommandBuffers(
      &m_command_buffers[image_index], &image_index);
  if (result != VK_SUCCESS)
  {
    throw std::runtime_error("FAILURE::can't present swap chain image!");
  }
}

static void sierpinski(
    std::vector<corevu::CoreVuModel::Vertex>& vertices, int depth,
    glm::vec2 left, glm::vec2 right, glm::vec2 top)
{
  if (depth <= 0)
  {
    vertices.push_back({top});
    vertices.push_back({right});
    vertices.push_back({left});
  }
  else
  {
    auto leftTop = 0.5f * (left + top);
    auto rightTop = 0.5f * (right + top);
    auto leftRight = 0.5f * (left + right);
    sierpinski(vertices, depth - 1, left, leftRight, leftTop);
    sierpinski(vertices, depth - 1, leftRight, right, rightTop);
    sierpinski(vertices, depth - 1, leftTop, rightTop, top);
  }
}

void TestApp::loadModels()
{
  // base solution
  // std::vector<corevu::CoreVuModel::Vertex> vertices{
  //     {{0.0f, -0.5f}}, {{0.5f, 0.5f}}, {{-0.5f, 0.5f}}};

  // sierpinski solution
  std::vector<corevu::CoreVuModel::Vertex> vertices{};
  sierpinski(vertices, 5, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.0f, -0.5f});

  m_corevu_model =
      std::make_unique<corevu::CoreVuModel>(m_corevu_device, vertices);
}
