#include "app.hpp"
#include <corevu/include/systems/render_system.hpp>
#include <corevu/include/corevu_camera.hpp>
#include <Tracy.hpp>

// temp libs
#define GLM_FORCE_RADIANS           // to be sure that no change depending on system
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // instead of -1 to 1 ?
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <chrono>
#include <thread>

using namespace corevutest;

const int FPS = 60;
const std::chrono::milliseconds FRAME_DURATION(1000 / FPS);

SampleApp::SampleApp()
{
  loadGameObjects();
}

SampleApp::~SampleApp()
{
}

void SampleApp::run()
{
  corevu::RenderSystem render_system{
      m_corevu_device, m_renderer.GetSwapchainRenderpass()};
  corevu::CoreVuCamera camera{};
  camera.setOrthographicProjection(-1, 1, -1, 1, -1, 1);

  while (!m_corevu_window.shouldClose())
  {
    ZoneScoped;
    const auto frame_start = std::chrono::steady_clock::now();

    glfwPollEvents(); // on some pltforms processing of events can block
                      // polling. The window refresh callback can be used to
                      // fix that.

    if (auto command_buffer = m_renderer.BeginFrame())
    {
      m_renderer.BeginSwapChainRenderPass(command_buffer);
      render_system.renderGameObjects(command_buffer, m_game_objects, camera);
      m_renderer.EndSwapChainRenderPass(command_buffer);
      m_renderer.EndFrame();
    }
    const auto frame_finish = std::chrono::steady_clock::now();
    const std::chrono::duration<double, std::milli> frame_time =
        frame_finish - frame_start;
    if (frame_time < FRAME_DURATION)
    {
      std::this_thread::sleep_for(FRAME_DURATION - frame_time);
    }

    FrameMark;
  }

  vkDeviceWaitIdle(m_corevu_device.device());
}

// temporary helper function, creates a 1x1x1 cube centered at offset
std::shared_ptr<corevu::CoreVuModel> createCubeModel(
    corevu::CoreVuDevice& device, glm::vec3 offset)
{
  std::vector<corevu::CoreVuModel::Vertex> vertices{

      // left face (white)
      {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

      // right face (yellow)
      {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .8f, .1f}},

      // top face (orange, remember y axis points down)
      {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

      // bottom face (red)
      {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .1f, .1f}},

      // nose face (blue)
      {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
      {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
      {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

      // tail face (green)
      {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
      {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
      {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

  };
  for (auto& v : vertices)
  {
    v.position += offset;
  }
  return std::make_shared<corevu::CoreVuModel>(device, vertices);
}

void SampleApp::loadGameObjects()
{
  // 3d solution
  auto model = createCubeModel(m_corevu_device, {.0f, .0f, .0f});
  auto cube = corevu::CoreVuGameObject::Create();
  cube.model = model;
  cube.transform.translation = {.0f, .0f, .5f};
  cube.transform.scale = {.5f, .5f, .5f};
  m_game_objects.push_back(std::move(cube));

  // base solution
  // {
  //   std::vector<corevu::CoreVuModel::Vertex> vertices{
  //       {{0.0f, -0.5f, 0.f}, {1.0f, 0.0f, 0.0f}},
  //       {{0.5f, 0.5f, 0.f}, {0.0f, 1.0f, 0.0f}},
  //       {{-0.5f, 0.5f, 0.f}, {0.0f, 0.0f, 1.0f}}};

  //   // sierpinski solution
  //   // std::vector<corevu::CoreVuModel::Vertex> vertices{};
  //   // sierpinski(vertices, 5, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.0f, -0.5f});

  //   auto corevu_model =
  //       std::make_shared<corevu::CoreVuModel>(m_corevu_device, vertices);

  //   auto triangle = corevu::CoreVuGameObject::Create();
  //   triangle.model = corevu_model;
  //   triangle.color = {.1f, .8f, .1f};
  //   triangle.transform.translation.x = .2f;
  //   triangle.transform.scale = {2.f, .5f, 0.f};
  //   triangle.transform.rotation.x = .25f * glm::two_pi<float>();

  //   m_game_objects.emplace_back(std::move(triangle));
  // }
}
