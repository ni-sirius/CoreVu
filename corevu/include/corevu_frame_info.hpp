#pragma once

#include <corevu_camera.hpp>
#include <corevu_gameobject.hpp>

// lib
#include <vulkan/vulkan.h>

namespace corevu
{
#define MAX_LIGHTS 10

struct PointLight
{
  glm::vec4 position; // ignore w
  glm::vec4 color;    // w is intensity
  // float range;
};

struct GlobalUbo
{
  /* NOTE it has the same alignment 16 bytes requirement as PushConstants */
  glm::mat4 projection_matrix{1.f}; // already 16 bytes aligned
  glm::mat4 view_matrix{1.f};       // already 16 bytes aligned
  glm::vec4 ambient_light_color{1.f, 1.f, 1.f, .02f};
  // glm::vec3 light_direction = glm::normalize(glm::vec3{1.f, -3.f, -1.f});
  PointLight point_lights[MAX_LIGHTS];
  int point_light_count{0};
};

struct FrameInfo
{
  int frame_index;
  float frame_time;
  VkCommandBuffer command_buffer;
  CoreVuCamera& camera;
  VkDescriptorSet global_descriptor_set;
  CoreVuGameObject::ObjectContainer& game_objects;
};

} // namespace corevu