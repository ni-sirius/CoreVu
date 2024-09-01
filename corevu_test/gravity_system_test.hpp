// #pragma once

// #include <corevu/include/corevu_device.hpp>
// #include <corevu/include/corevu_gameobject.hpp>
// #include <corevu/include/corevu_window.hpp>
// #include <corevu/include/systems/render_system.hpp>
// #include <corevu/include/systems/gravity_system.hpp>
// #include <corevu/include/systems/vector_field_system.hpp>
// #include "renderer.hpp"

// // std
// #include <cstdlib>
// #include <iostream>
// #include <stdexcept>
// #include <memory>

// namespace corevutest
// {

// class GravitySystemTestApp
// {
// public:
//   static constexpr int width = 800;
//   static constexpr int height = 600;

//   GravitySystemTestApp()
//   {
//     loadGameObjects();
//   };
//   ~GravitySystemTestApp()
//   {

//   };
//   GravitySystemTestApp(const GravitySystemTestApp&) = delete;
//   GravitySystemTestApp& operator=(const GravitySystemTestApp&) = delete;

//   void run()
//   {
//     // create some models
//     std::shared_ptr<corevu::CoreVuModel> squareModel =
//         corevu::createSquareModel(
//             m_corevu_device,
//             {.5f, .0f}); // offset model by .5 so rotation occurs at edge
//                          // rather than center of square
//     std::shared_ptr<corevu::CoreVuModel> circleModel =
//         corevu::createCircleModel(m_corevu_device, 64);

//     // create physics objects
//     std::vector<corevu::CoreVuGameObject> physicsObjects{};
//     auto red = corevu::CoreVuGameObject::Create();
//     red.transform.scale = glm::vec2{.05f};
//     red.transform.translation = {.5f, .5f};
//     red.color = {1.f, 0.f, 0.f};
//     red.rigid_body.velocity = {-.5f, .0f};
//     red.model = circleModel;
//     physicsObjects.push_back(std::move(red));
//     auto blue = corevu::CoreVuGameObject::Create();
//     blue.transform.scale = glm::vec2{.05f};
//     blue.transform.translation = {-.45f, -.25f};
//     blue.color = {0.f, 0.f, 1.f};
//     blue.rigid_body.velocity = {.5f, .0f};
//     blue.model = circleModel;
//     physicsObjects.push_back(std::move(blue));

//     // create vector field
//     std::vector<corevu::CoreVuGameObject> vectorField{};
//     int gridCount = 40;
//     for (int i = 0; i < gridCount; i++)
//     {
//       for (int j = 0; j < gridCount; j++)
//       {
//         auto vf = corevu::CoreVuGameObject::Create();
//         vf.transform.scale = glm::vec2(0.005f);
//         vf.transform.translation = {
//             -1.0f + (i + 0.5f) * 2.0f / gridCount,
//             -1.0f + (j + 0.5f) * 2.0f / gridCount};
//         vf.color = glm::vec3(1.0f);
//         vf.model = squareModel;
//         vectorField.push_back(std::move(vf));
//       }
//     }

//     corevu::GravityPhysicsSystem gravitySystem{0.81f};
//     corevu::Vec2FieldSystem vecFieldSystem{};

//     corevu::RenderSystem simpleRenderSystem{
//         m_corevu_device, m_renderer.GetSwapchainRenderpass()};

//     while (!m_corevu_window.shouldClose())
//     {
//       glfwPollEvents();

//       if (auto commandBuffer = m_renderer.BeginFrame())
//       {
//         // update systems
//         gravitySystem.update(physicsObjects, 1.f / 60, 5);
//         vecFieldSystem.update(gravitySystem, physicsObjects, vectorField);

//         // render system
//         m_renderer.BeginSwapChainRenderPass(commandBuffer);
//         simpleRenderSystem.renderGameObjects(commandBuffer, physicsObjects);
//         simpleRenderSystem.renderGameObjects(commandBuffer, vectorField);
//         m_renderer.EndSwapChainRenderPass(commandBuffer);
//         m_renderer.EndFrame();
//       }
//     }

//     vkDeviceWaitIdle(m_corevu_device.device());
//   }

// private:
//   void loadGameObjects()
//   {
//     std::vector<corevu::CoreVuModel::Vertex> vertices{
//         {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
//         {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
//         {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};
//     auto lveModel =
//         std::make_shared<corevu::CoreVuModel>(m_corevu_device, vertices);

//     auto triangle = corevu::CoreVuGameObject::Create();
//     triangle.model = lveModel;
//     triangle.color = {.1f, .8f, .1f};
//     triangle.transform.translation.x = .2f;
//     triangle.transform.scale = {2.f, .5f};
//     triangle.transform.rotation = .25f * glm::two_pi<float>();

//     m_game_objects.push_back(std::move(triangle));
//   }

// private:
//   corevu::CoreVuWindow m_corevu_window{
//       width, height, "CoreVu::GravitySystemTestApp"};
//   corevu::CoreVuDevice m_corevu_device{m_corevu_window};
//   SampleRenderer m_renderer{m_corevu_window, m_corevu_device};

//   std::vector<corevu::CoreVuGameObject>
//       m_game_objects; // or // std::unique_ptr<corevu::CoreVuModel>
//                       // m_corevu_model;
// };
// } // namespace corevutest