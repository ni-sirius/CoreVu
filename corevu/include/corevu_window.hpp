#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>

#include <string>

namespace corevu
{
class CoreVuWindow
{
public:
  CoreVuWindow(int width, int height, const std::string& name);
  ~CoreVuWindow();

  CoreVuWindow(const CoreVuWindow&) = delete;
  CoreVuWindow& operator=(const CoreVuWindow&) = delete;

  void CreateWindowSurface(VkInstance& ins, VkSurfaceKHR* sur);

  bool shouldClose()
  {
    return glfwWindowShouldClose(m_window);
  }

  VkExtent2D GetExtent()
  {
    return {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height)};
  }

private:
  void initWindow();

private:
  const int m_width;
  const int m_height;
  std::string m_window_name;

  GLFWwindow* m_window = nullptr;
};
} // namespace corevu