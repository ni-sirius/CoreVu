#include "app.hpp"
#include "render_system.hpp"

// temp libs
#define GLM_FORCE_RADIANS           // to be sure that no change depending on system
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // instead of -1 to 1 ?
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
using namespace corevutest;

SampleApp::SampleApp()
{
  loadGameObjects();
}

SampleApp::~SampleApp()
{
}

void SampleApp::run()
{
  RenderSystem render_system{
      m_corevu_device, m_renderer.GetSwapchainRenderpass()};

  while (!m_corevu_window.shouldClose())
  {
    glfwPollEvents(); // on some pltforms processing of events can block
                      // polling. The window refresh callback can be used to
                      // fix that.

    if (auto command_buffer = m_renderer.BeginFrame())
    {
      m_renderer.BeginSwapChainRenderPass(command_buffer);
      render_system.renderGameObjects(command_buffer, m_game_objects);
      m_renderer.EndSwapChainRenderPass(command_buffer);
      m_renderer.EndFrame();
    }
  }

  vkDeviceWaitIdle(m_corevu_device.device());
}

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
