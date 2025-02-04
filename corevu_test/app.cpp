#include "app.hpp"
#include <corevu/include/ext/keyboard_movement_controller.hpp>
#include <corevu/include/systems/render_system.hpp>
#include <corevu/include/systems/point_light_system.hpp>
#include <corevu/include/systems/texture_render_system.hpp>
#include <corevu/include/corevu_camera.hpp>
#include <corevu/include/corevu_buffer.hpp>

#include <corevu/include/corevu_frame_info.hpp> // to re-think, because the uniform description shouldn't be part of the engine

// libs
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
  m_global_descriptor_pool =
      corevu::CoreVuDescriptorPool::Builder(m_corevu_device)
          .setMaxSets(corevu::CoreVuSwapChain::MAX_FRAMES_IN_FLIGHT)
          .addPoolSize(
              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
              corevu::CoreVuSwapChain::MAX_FRAMES_IN_FLIGHT)
          .build(); // why both amount of pools and amount of desctiptors per
                    // set is MAX_FRAMES_IN_FLIGHT? Because having 2 sets
                    // doesn't mean that there will be 1 descriptor in each, you
                    // need to specify overall count of descriptors in the pool.

  // build frame descriptor pools
  m_frame_pools.resize(corevu::CoreVuSwapChain::MAX_FRAMES_IN_FLIGHT);
  auto frame_pool_builder =
      corevu::CoreVuDescriptorPool::Builder::Builder(m_corevu_device)
          .setMaxSets(1000)
          .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
          .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
          .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
  for (int i = 0; i < m_frame_pools.size(); i++)
  {
    m_frame_pools[i] = frame_pool_builder.build();
  }

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
        m_corevu_device, sizeof(corevu::GlobalUbo), 1,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
        /* | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT could be used but we will go for using flush()*/);
    uniform_buffer->map();
  }

  auto global_descriptor_set_layout =
      corevu::CoreVuDescriptorSetLayout::Builder(m_corevu_device)
          .addBinding(
              0,
              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS 
              /* alternative VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT */)
          .build();

  std::vector<VkDescriptorSet> global_descriptor_sets{
      corevu::CoreVuSwapChain::MAX_FRAMES_IN_FLIGHT};
  for (int i = 0; i < corevu::CoreVuSwapChain::MAX_FRAMES_IN_FLIGHT; i++)
  {
    auto buffer_info = uniform_buffers[i]->descriptorInfo();
    corevu::CoreVuDescriptorWriter(
        *global_descriptor_set_layout, *m_global_descriptor_pool)
        .writeBuffer(0, &buffer_info)
        .build(global_descriptor_sets[i]);
  }

  corevu::RenderSystem render_system{
      m_corevu_device, m_renderer.GetSwapchainRenderpass(),
      global_descriptor_set_layout->getDescriptorSetLayout()};
  corevu::PointLightSystem point_light_system{
      m_corevu_device, m_renderer.GetSwapchainRenderpass(),
      global_descriptor_set_layout->getDescriptorSetLayout()};
  corevu::TextureRenderSystem texture_render_system{
      m_corevu_device, m_renderer.GetSwapchainRenderpass(),
      global_descriptor_set_layout->getDescriptorSetLayout()};

  corevu::CoreVuCamera camera{};
  // camera.setViewDirection(glm::vec3(0.f), glm::vec3(0.5f, 0.f, 1.f));
  // camera.setViewTarget(glm::vec3(1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

  auto viewer_object = corevu::CoreVuGameObject::Create();
  viewer_object.transform.translation = {0.f, 0.f, -2.5f};
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
        glm::radians(50.f), aspect_ratio, 0.1f, 100.f);

    if (auto command_buffer = m_renderer.BeginFrame())
    {
      const int frame_index = m_renderer.GetFrameIndex();
      m_frame_pools[frame_index]->resetPool();
      corevu::FrameInfo frame_info{
          .frame_index = frame_index,
          .frame_time = dt_sec,
          .command_buffer = command_buffer,
          .camera = camera,
          .global_descriptor_set = global_descriptor_sets[frame_index],
          .frame_descriptor_pool = *m_frame_pools[frame_index],
          .game_objects = m_game_objects};

      // update
      corevu::GlobalUbo ubo{};
      ubo.projection_matrix = camera.getProjection();
      ubo.view_matrix = camera.getView();
      ubo.inverse_view_matrix = camera.getInverseView();
      point_light_system.update(
          frame_info, ubo); // before writing to ubo buffers below
      uniform_buffers[frame_index]->writeToBuffer(&ubo);
      uniform_buffers[frame_index]->flush();

      // render
      m_renderer.BeginSwapChainRenderPass(command_buffer);

      // oredered by transparency
      texture_render_system.renderGameObjects(frame_info);
      render_system.renderGameObjects(frame_info);
      point_light_system.render(frame_info);

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

  // First object
  {
    auto model = corevu::CoreVuModel::CreateModelFromPath(
        m_corevu_device,
        "C:\\workspace\\CoreVu\\assets\\models\\smooth_vase.obj");
    // createCubeModel(m_corevu_device, {.0f, .0f, .0f});
    auto object = corevu::CoreVuGameObject::Create();
    object.model = model;
    object.transform.translation = {
        1.0f, .5f, 0}; // z 2.5 for perspective, 0.5f for othographic (look in
                       // +z direction)
    object.transform.scale = {2.5f, 1.5f, 2.5f};

    const auto obj_id = object.GetUid();
    m_game_objects.emplace(obj_id, std::move(object));
  }

  // Second object
  {
    auto model = corevu::CoreVuModel::CreateModelFromPath(
        m_corevu_device,
        "C:\\workspace\\CoreVu\\assets\\models\\flat_vase.obj");
    // createCubeModel(m_corevu_device, {.0f, .0f, .0f});
    auto object = corevu::CoreVuGameObject::Create();
    object.model = model;
    object.transform.translation = {
        -1.0f, .5f, 0}; // z 2.5 for perspective, 0.5f for othographic (look in
                        // +z direction)
    object.transform.scale = {2.5f, 1.5f, 2.5f};

    const auto obj_id = object.GetUid();
    m_game_objects.emplace(obj_id, std::move(object));
  }

  // Third object
  {
    auto model = corevu::CoreVuModel::CreateModelFromPath(
        m_corevu_device,
        "C:\\workspace\\CoreVu\\assets\\models\\colored_cube.obj");
    // createCubeModel(m_corevu_device, {.0f, .0f, .0f});
    auto object = corevu::CoreVuGameObject::Create();
    object.model = model;
    object.transform.translation = {
        0.0f, .0f, 0}; // z 2.5 for perspective, 0.5f for othographic (look in
                       // +z direction)
    object.transform.scale = {.25f, .25f, .25f};
    object.transform.rotation = {
        .13f * glm::two_pi<float>(), .13f * glm::two_pi<float>(), 0.f};

    const auto obj_id = object.GetUid();
    m_game_objects.emplace(obj_id, std::move(object));
  }

  // Floor object
  {
    auto model = corevu::CoreVuModel::CreateModelFromPath(
        m_corevu_device, "C:\\workspace\\CoreVu\\assets\\models\\quad.obj");
    auto texture = corevu::CoreVuTexture::createTextureFromPath(
        m_corevu_device, "C:\\workspace\\CoreVu\\assets\\textures\\missing.png");
    auto object = corevu::CoreVuGameObject::Create();
    object.model = model;
    object.diffuse_map = texture;
    object.transform.translation = {0.0f, .5f, 0};
    object.transform.scale = {3.f, 1.f, 3.f};

    const auto obj_id = object.GetUid();
    m_game_objects.emplace(obj_id, std::move(object));
  }

  // Point light object 1
  {
    auto object = corevu::CoreVuGameObject::CreateAsPointLight(0.2f);
    object.transform.translation = {0.0f, -1.0f, 0.0f};

    const auto obj_id = object.GetUid();
    m_game_objects.emplace(obj_id, std::move(object));
  }

  // Multi-point light generation
  {
    std::vector<glm::vec3> light_colors{
        {1.f, .1f, .1f}, {.1f, .1f, 1.f}, {.1f, 1.f, .1f},
        {1.f, 1.f, .1f}, {.1f, 1.f, 1.f}, {1.f, 1.f, 1.f} //
    };
    for (size_t i = 0; i < light_colors.size(); i++)
    {
      auto object = corevu::CoreVuGameObject::CreateAsPointLight(
          0.2f, (i + 1) * 0.1f, light_colors[i]);
      auto rotation = glm::rotate(
          glm::mat4(1.f), (i * glm::two_pi<float>()) / light_colors.size(),
          glm::vec3{0.f, -1.f, 0.f});
      object.transform.translation =
          glm::vec3{rotation * glm::vec4{-1.f, -1.f, -1.f, 1.f}};

      const auto obj_id = object.GetUid();
      m_game_objects.emplace(obj_id, std::move(object));
    }
  }

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
