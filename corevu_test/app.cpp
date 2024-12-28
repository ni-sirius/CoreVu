#include "app.hpp"
#include <corevu/include/ext/keyboard_movement_controller.hpp>
#include <corevu/include/systems/render_system.hpp>
#include <corevu/include/corevu_camera.hpp>
#include <corevu/include/corevu_buffer.hpp>
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

struct GlobalUbo
{
  glm::mat4 projection_view_matrix{1.f};
  glm::vec3 light_direction = glm::normalize(glm::vec3{1.f, -3.f, -1.f});
};

SampleApp::SampleApp()
{
  loadGameObjects();
}

SampleApp::~SampleApp()
{
}

void SampleApp::run()
{
  // setting up uniforms for the app
  std::vector<std::unique_ptr<corevu::CoreVuBuffer>> uniform_buffers(
      corevu::CoreVuSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (auto& uniform_buffer : uniform_buffers)
  {
    uniform_buffer = std::make_unique<corevu::CoreVuBuffer>(
        m_corevu_device, sizeof(GlobalUbo), 1,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
        /* | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT could be used but we will go for using flush()*/);
    uniform_buffer->map();
  }

  corevu::RenderSystem render_system{
      m_corevu_device, m_renderer.GetSwapchainRenderpass()};
  corevu::CoreVuCamera camera{};
  // camera.setViewDirection(glm::vec3(0.f), glm::vec3(0.5f, 0.f, 1.f));
  // camera.setViewTarget(glm::vec3(1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

  auto viewer_object = corevu::CoreVuGameObject::Create();
  corevu::KeyboardMovementController keyboard_camera_controller{};

  auto frame_start = std::chrono::steady_clock::now();
  auto current_time = frame_start;
  while (!m_corevu_window.shouldClose())
  {
    ZoneScoped;

    glfwPollEvents(); // on some pltforms processing of events can block
                      // polling. The window refresh callback can be used to
                      // fix that.

    auto now = std::chrono::steady_clock::now();
    const auto dt_sec =
        std::chrono::duration<float, std::chrono::seconds::period>(
            now - current_time)
            .count();
    current_time = now;

    keyboard_camera_controller.moveInPlaneXZ(
        m_corevu_window.GetGLFWwindow(), dt_sec,
        viewer_object); // camera position preserve in viewerobj.
    camera.setViewYXZ(
        viewer_object.transform.translation, viewer_object.transform.rotation);

    // NOTE compensate stretching of Vulkan viewport to swapchain
    // renderbuffersize with aspect_ratio in projection matrix
    const float aspect_ratio = m_renderer.GetAspectRatio();
    // camera.setOrthographicProjection(-aspect_ratio, aspect_ratio, -1, 1, -1,
    // 1); //bottom and top shall be -1 1
    camera.setPerspectiveProjection(
        glm::radians(50.f), aspect_ratio, 0.1f, 10.f);

    if (auto command_buffer = m_renderer.BeginFrame())
    {
      const int frame_index = m_renderer.GetFrameIndex();
      corevu::FrameInfo frame_info{frame_index, dt_sec, command_buffer, camera};

      // update
      GlobalUbo ubo{};
      ubo.projection_view_matrix = camera.getProjection() * camera.getView();
      uniform_buffers[frame_index]->writeToBuffer(&ubo);
      uniform_buffers[frame_index]->flush();

      // render
      m_renderer.BeginSwapChainRenderPass(command_buffer);
      render_system.renderGameObjects(frame_info, m_game_objects);
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
    frame_start = std::chrono::steady_clock::now();

    FrameMark;
  }

  vkDeviceWaitIdle(m_corevu_device.device());
}

// temporary helper function, creates a 1x1x1 cube centered at offset
std::shared_ptr<corevu::CoreVuModel> createCubeModel(
    corevu::CoreVuDevice& device, glm::vec3 offset)
{
  corevu::CoreVuModel::Builder builder{};
  builder.vertices = {
      // left face (white)
      {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},

      // right face (yellow)
      {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},

      // top face (orange, remember y axis points down)
      {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},

      // bottom face (red)
      {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},

      // nose face (blue)
      {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
      {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},

      // tail face (green)
      {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
      {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
  };

  builder.indices = {0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,
                     8,  9,  10, 8,  11, 9,  12, 13, 14, 12, 15, 13,
                     16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21};

  for (auto& v : builder.vertices)
  {
    v.position += offset;
  }

  return std::make_shared<corevu::CoreVuModel>(device, builder);
}

void SampleApp::loadGameObjects()
{
  // 3d solution
  auto model = corevu::CoreVuModel::CreateModelFromPath(
      m_corevu_device,
      "C:\\workspace\\CoreVu\\assets\\models\\smooth_vase.obj");
  // createCubeModel(m_corevu_device, {.0f, .0f, .0f});
  auto object = corevu::CoreVuGameObject::Create();
  object.model = model;
  object.transform.translation = {
      .0f, .0f, 2.5f}; // z 2.5 for perspective, 0.5f for othographic (look in
                       // +z direction)
  object.transform.scale = {2.5f, .5f, 2.5f};
  m_game_objects.push_back(std::move(object));

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
