#pragma once

#include <corevu_camera.hpp>
#include <corevu_gameobject.hpp>

// lib
#include <vulkan/vulkan.h>

namespace corevu
{
  struct FrameInfo
  {
    int frame_index;
    float frame_time;
    VkCommandBuffer command_buffer;
    CoreVuCamera& camera;
    VkDescriptorSet global_descriptor_set;
    CoreVuGameObject::ObjectContainer& game_objects;
  };
  
}