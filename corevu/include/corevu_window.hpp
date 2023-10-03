#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>

namespace corevu
{
class CoreVuWindow
{

public:
  void createWindowSurface(VkInstance& ins, VkSurfaceKHR* sur)
  {
  }

private:
  GLFWwindow* m_window;
};
} // namespace corevu