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
  bool WasWindowResized() {return m_framebuffer_is_resized;}
  void ResetWindowResized() {m_framebuffer_is_resized = false;}

private:
  static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
  void initWindow();

private:
  int m_width;
  int m_height;
  bool m_framebuffer_is_resized{false};

  std::string m_window_name;

  GLFWwindow* m_window = nullptr;
};
} // namespace corevu