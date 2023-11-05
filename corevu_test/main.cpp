#include <corevu/include/corevu_window.hpp>
#include <corevu/include/corevu_pipeline.hpp>

#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace corevutest
{

class TestApp
{
public:
  static constexpr int width = 800;
  static constexpr int height = 600;

  void run()
  {
    while (!m_corevu_window.shouldClose())
    {
      glfwPollEvents();
    }
  }

private:
  corevu::CoreVuWindow m_corevu_window{width, height, "hello world!"};
  corevu::CoreVuDevice m_corevu_device{m_corevu_window};
  corevu::CoreVuPipeline m_corevu_pipeline{
      m_corevu_device,
      corevu::CoreVuPipeline::DefaultPipelineConfigInfo(width, height),
      "../corevu/shaders/simple_shader.vert.spv",
      "../corevu/shaders/simple_shader.frag.spv"};
};
} // namespace corevutest

int main(int, char**)
{
  corevutest::TestApp app{};

  try
  {
    app.run();
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  std::cout << "Hello, from CoreVu!\n";
  return EXIT_SUCCESS;
}