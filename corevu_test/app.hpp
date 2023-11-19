#include <corevu/include/corevu_window.hpp>
#include <corevu/include/corevu_pipeline.hpp>
#include <corevu/include/corevu_swap_chain.hpp>
#include <corevu/include/corevu_model.hpp>

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <memory>

namespace corevutest
{

class TestApp
{
public:
  static constexpr int width = 800;
  static constexpr int height = 600;

  TestApp();
  ~TestApp();
  TestApp(const TestApp&) = delete;
  TestApp& operator=(const TestApp&) = delete;

  void run()
  {
    while (!m_corevu_window.shouldClose())
    {
      glfwPollEvents();
      drawFrame();
    }

    vkDeviceWaitIdle(m_corevu_device.device());
  }

private:
  void createPipelineLayout();
  void createPipeline();
  void createCommandBuffers();
  void drawFrame();

  void loadModels();

private:
  corevu::CoreVuWindow m_corevu_window{width, height, "hello world!"};
  corevu::CoreVuDevice m_corevu_device{m_corevu_window};
  corevu::CoreVuSwapChain m_corevu_swapchain{
      m_corevu_device, m_corevu_window.GetExtent()};

  std::unique_ptr<corevu::CoreVuPipeline> m_corevu_pipeline;
  VkPipelineLayout m_pipeline_layout;
  std::vector<VkCommandBuffer> m_command_buffers;
  // corevu::CoreVuPipeline m_corevu_pipeline{
  //     m_corevu_device,
  //     corevu::CoreVuPipeline::DefaultPipelineConfigInfo(width, height),
  //     "../corevu/shaders/simple_shader.vert.spv",
  //     "../corevu/shaders/simple_shader.frag.spv"};

  std::unique_ptr<corevu::CoreVuModel> m_corevu_model;
};
} // namespace corevutest