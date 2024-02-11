#pragma once
#include <corevu/include/corevu_window.hpp>
#include <corevu/include/corevu_swap_chain.hpp>

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <memory>

namespace corevutest
{

class SampleRenderer
{
public:
  SampleRenderer(corevu::CoreVuWindow& window, corevu::CoreVuDevice& device);
  ~SampleRenderer();
  SampleRenderer(const SampleRenderer&) = delete;
  SampleRenderer& operator=(const SampleRenderer&) = delete;

  VkCommandBuffer BeginFrame();
  void EndFrame();
  bool IsFrameInProgress() const
  {
    return m_is_frame_started;
  }

  void BeginSwapChainRenderPass(VkCommandBuffer command_buffer);
  void EndSwapChainRenderPass(VkCommandBuffer command_buffer);

  VkCommandBuffer GetCurrentCommandbuffer() const
  {
    assert(
        m_is_frame_started &&
        "FAILURE::cannot get command buffer when frame not in progress.");
    return m_command_buffers[m_current_frame_index];
  }

  int GetFrameIndex() const
  {
    assert(
        m_is_frame_started &&
        "FAILURE::cannot get frame index when frame not in progress.");

    return m_current_frame_index;
  }

  VkRenderPass GetSwapchainRenderpass() const
  {
    return m_corevu_swapchain->getRenderPass();
  }

private:
  void createCommandBuffers();
  void freeCommandBuffers();
  void recreateSwapchain();

private:
  corevu::CoreVuWindow& m_corevu_window;
  corevu::CoreVuDevice& m_corevu_device;
  std::unique_ptr<corevu::CoreVuSwapChain> m_corevu_swapchain{nullptr};
  std::vector<VkCommandBuffer> m_command_buffers;

  uint32_t m_current_image_index = 0;
  int m_current_frame_index = 0;
  bool m_is_frame_started = false;
};
} // namespace corevutest