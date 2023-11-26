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
      glfwPollEvents(); // on some pltforms processing of events can block
                        // polling. The window refresh callback can be used to
                        // fix that.
      drawFrame();
    }

    vkDeviceWaitIdle(m_corevu_device.device());
  }

private:
  void createPipelineLayout();
  void createPipeline();
  void createCommandBuffers();
  void freeCommandBuffers();
  void drawFrame();

  void loadModels();

  void recreateSwapchain();
  void recordCommandBuffer(int imageIndex);

private:
  corevu::CoreVuWindow m_corevu_window{width, height, "hello world!"};
  corevu::CoreVuDevice m_corevu_device{m_corevu_window};
  std::unique_ptr<corevu::CoreVuSwapChain> m_corevu_swapchain{nullptr};
  std::unique_ptr<corevu::CoreVuPipeline> m_corevu_pipeline{nullptr};
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