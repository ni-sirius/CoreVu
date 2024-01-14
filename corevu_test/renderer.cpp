#include "renderer.hpp"

// std
#include <array>

using namespace corevutest;

SampleRenderer::SampleRenderer(
    corevu::CoreVuWindow& window, corevu::CoreVuDevice& device)
  : m_corevu_window{window}, m_corevu_device{device}
{
  recreateSwapchain();
  createCommandBuffers();
}

SampleRenderer::~SampleRenderer()
{
  freeCommandBuffers();
}

VkCommandBuffer SampleRenderer::BeginFrame()
{
  assert(
      !m_is_frame_started &&
      "FAILURE::cannot begin frame while it's already in progress.");

  auto result = m_corevu_swapchain->acquireNextImage(&m_current_image_index);

  if (result == VK_ERROR_OUT_OF_DATE_KHR)
  {
    recreateSwapchain();
    return nullptr;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
  {
    throw std::runtime_error("FAILURE:: can't acuire swap chain image!");
  }

  m_is_frame_started = true;

  auto command_buffer = GetCurrentCommandbuffer();
  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS)
  {
    throw std::runtime_error("FAILURE::can't begin recording frame buffer!");
  }
  return command_buffer;
}

void SampleRenderer::EndFrame()
{
  assert(
      m_is_frame_started &&
      "FAILURE::cannot end frame while it's not in progress.");

  auto command_buffer = GetCurrentCommandbuffer();
  if (vkEndCommandBuffer(command_buffer))
  {
    std::runtime_error("FAILURE::can't end recording of frame buffer!");
  }

  auto result = m_corevu_swapchain->submitCommandBuffers(
      &command_buffer, &m_current_image_index);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      m_corevu_window.WasWindowResized())
  {
    m_corevu_window.ResetWindowResized();
    recreateSwapchain();
  }
  else if (result != VK_SUCCESS)
  {
    throw std::runtime_error("FAILURE::can't present swap chain image!");
  }

  m_is_frame_started = false;
}

void SampleRenderer::BeginSwapChainRenderPass(VkCommandBuffer command_buffer)
{
  assert(
      m_is_frame_started && "FAILURE::cannot call BeginSwapChainRenderPass if "
                            "frame is not in progress.");
  assert(
      command_buffer == GetCurrentCommandbuffer() &&
      "FAILURE::cannot begin render pass on command buffer from different "
      "frame.");

  VkRenderPassBeginInfo render_pass_info{};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_info.renderPass = m_corevu_swapchain->getRenderPass();
  render_pass_info.framebuffer =
      m_corevu_swapchain->getFrameBuffer(m_current_image_index);

  render_pass_info.renderArea.offset = {0, 0};
  render_pass_info.renderArea.extent = m_corevu_swapchain->getSwapChainExtent();

  // it's specified in corevu_swap_chain that first attachement is color and
  // second is depth
  std::array<VkClearValue, 2> clear_values{};
  clear_values[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
  clear_values[1].depthStencil = {1.0f, 0};
  render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
  render_pass_info.pClearValues = clear_values.data();

  vkCmdBeginRenderPass(
      command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width =
      static_cast<float>(m_corevu_swapchain->getSwapChainExtent().width);
  viewport.height =
      static_cast<float>(m_corevu_swapchain->getSwapChainExtent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  VkRect2D scissor{{0, 0}, m_corevu_swapchain->getSwapChainExtent()};
  vkCmdSetViewport(command_buffer, 0, 1, &viewport);
  vkCmdSetScissor(command_buffer, 0, 1, &scissor);
}

void SampleRenderer::EndSwapChainRenderPass(VkCommandBuffer command_buffer)
{
  assert(
      m_is_frame_started && "FAILURE::cannot call EndSwapChainRenderPass if "
                            "frame is not in progress.");
  assert(
      command_buffer == GetCurrentCommandbuffer() &&
      "FAILURE::cannot end render pass on command buffer from different "
      "frame.");

  vkCmdEndRenderPass(command_buffer);
}

void SampleRenderer::createCommandBuffers()
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

void SampleRenderer::freeCommandBuffers()
{
  vkFreeCommandBuffers(
      m_corevu_device.device(), m_corevu_device.getCommandPool(),
      static_cast<uint32_t>(m_command_buffers.size()),
      m_command_buffers.data());

  m_command_buffers.clear();
}

void SampleRenderer::recreateSwapchain()
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

    std::shared_ptr<corevu::CoreVuSwapChain> old_swap_chain =
        std::move(m_corevu_swapchain);
    m_corevu_swapchain = std::make_unique<corevu::CoreVuSwapChain>(
        m_corevu_device, extent, old_swap_chain);

    if (!old_swap_chain->compareSwapFormats(*m_corevu_swapchain.get()))
    {
      throw std::runtime_error(
          "FAILURE::two incompatible swapchains are created!"); // TODO callback for the main app.
    }

    if (m_corevu_swapchain &&
        m_corevu_swapchain->imageCount() != m_command_buffers.size())
    {
      freeCommandBuffers();
      createCommandBuffers();
    }
  }

  // #error - no pipeline recreated - to be changed/fixed later
  //  createPipeline(); // if render passes are compatible (same layout of
  //  data),
  //   the pipline can stay the same
}
