#include "app.hpp"

// std
#include <array>
using namespace corevutest;

TestApp::TestApp()
{
  loadModels();
  createPipelineLayout();
  recreateSwapchain();
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
  assert(m_corevu_swapchain != nullptr && "Cannot create pipeline before swap chain");
  assert(m_pipeline_layout != nullptr && "Cannot create pipeline before pipeline layout");

  corevu::PipelineConfigInfo pipeline_config{};
  corevu::CoreVuPipeline::DefaultPipelineConfigInfo(pipeline_config);
  pipeline_config.renderPass = m_corevu_swapchain->getRenderPass();
  pipeline_config.pipelineLayout = m_pipeline_layout;
  m_corevu_pipeline = std::make_unique<corevu::CoreVuPipeline>(
      m_corevu_device, pipeline_config,
      "../corevu/shaders/simple_shader.vert.spv",
      "../corevu/shaders/simple_shader.frag.spv");
}

void TestApp::createCommandBuffers()
{
  m_command_buffers.resize(
      m_corevu_swapchain
          ->imageCount()); // one to one command bufer - swap chain

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
}

void TestApp::freeCommandBuffers()
{
  vkFreeCommandBuffers(
      m_corevu_device.device(), m_corevu_device.getCommandPool(),
      static_cast<uint32_t>(m_command_buffers.size()),
      m_command_buffers.data());

  m_command_buffers.clear();
}

void TestApp::drawFrame()
{
  uint32_t image_index;
  auto result = m_corevu_swapchain->acquireNextImage(&image_index);

  if (result == VK_ERROR_OUT_OF_DATE_KHR)
  {
    recreateSwapchain();
    return;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
  {
    throw std::runtime_error("FAILURE:: can't acuire swap chain image!");
  }

  recordCommandBuffer(image_index);
  result = m_corevu_swapchain->submitCommandBuffers(
      &m_command_buffers[image_index], &image_index);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      m_corevu_window.WasWindowResized())
  {
    m_corevu_window.ResetWindowResized();
    recreateSwapchain();
    return;
  }
  else if (result != VK_SUCCESS)
  {
    throw std::runtime_error("FAILURE::can't present swap chain image!");
  }
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

void TestApp::loadModels()
{
  // base solution
  std::vector<corevu::CoreVuModel::Vertex> vertices{
      {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
      {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

  // sierpinski solution
  // std::vector<corevu::CoreVuModel::Vertex> vertices{};
  // sierpinski(vertices, 5, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.0f, -0.5f});

  m_corevu_model =
      std::make_unique<corevu::CoreVuModel>(m_corevu_device, vertices);
}

void TestApp::recreateSwapchain()
{
  auto extent = m_corevu_window.GetExtent();
  while (extent.width == 0 || extent.height == 0)
  {
    extent = m_corevu_window.GetExtent();
    glfwWaitEvents();
  }
  vkDeviceWaitIdle(m_corevu_device.device());

  if (m_corevu_swapchain == nullptr)
  {
    m_corevu_swapchain =
        std::make_unique<corevu::CoreVuSwapChain>(m_corevu_device, extent);
  }
  else
  {
    // m_corevu_swapchain =
    //     nullptr; // necessary because/when driver doesn't allow to have two
    //     swapchains
    //              // per screen. On make_unique first the swapchain is created
    //              and
    //              // only after the old one is deleted.

    m_corevu_swapchain = std::make_unique<corevu::CoreVuSwapChain>(
        m_corevu_device, extent, std::move(m_corevu_swapchain));
    if (m_corevu_swapchain &&
        m_corevu_swapchain->imageCount() != m_command_buffers.size())
    {
      freeCommandBuffers();
      createCommandBuffers();
    }
  }

  createPipeline(); // if render passes are compatible (same layout of data),
                    // the pipline can stay the same
}

void TestApp::recordCommandBuffer(int imageIndex)
{
  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(m_command_buffers[imageIndex], &begin_info) !=
      VK_SUCCESS)
  {
    throw std::runtime_error("FAILURE::can't begin recording frame buffer!");
  }

  VkRenderPassBeginInfo render_pass_info{};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_info.renderPass = m_corevu_swapchain->getRenderPass();
  render_pass_info.framebuffer = m_corevu_swapchain->getFrameBuffer(imageIndex);

  render_pass_info.renderArea.offset = {0, 0};
  render_pass_info.renderArea.extent = m_corevu_swapchain->getSwapChainExtent();

  // it's specified in corevu_swap_chain that first attachement is color and
  // second is depth
  std::array<VkClearValue, 2> clear_values{};
  clear_values[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
  clear_values[1].depthStencil = {1.0f, 0};
  render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
  render_pass_info.pClearValues = clear_values.data();

  vkCmdBeginRenderPass(
      m_command_buffers[imageIndex], &render_pass_info,
      VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(m_corevu_swapchain->getSwapChainExtent().width);
  viewport.height = static_cast<float>(m_corevu_swapchain->getSwapChainExtent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  VkRect2D scissor{{0, 0}, m_corevu_swapchain->getSwapChainExtent()};
  vkCmdSetViewport(m_command_buffers[imageIndex], 0, 1, &viewport);
  vkCmdSetScissor(m_command_buffers[imageIndex], 0, 1, &scissor);

  m_corevu_pipeline->Bind(m_command_buffers[imageIndex]);
  // vkCmdDraw(m_command_buffers[imageIndex], 3, 1, 0, 0); // for hardcoded
  // implementation
  m_corevu_model->Bind(m_command_buffers[imageIndex]);
  m_corevu_model->Draw(m_command_buffers[imageIndex]);

  vkCmdEndRenderPass(m_command_buffers[imageIndex]);
  if (vkEndCommandBuffer(m_command_buffers[imageIndex]))
  {
    std::runtime_error("FAILURE::can't end recording of frame buffer!");
  }
}
