#include <corevu_window.hpp>
#include <iostream>

using namespace corevu;

CoreVuWindow::CoreVuWindow(int width, int height, const std::string& name)
  : m_width{width}, m_height{height}, m_window_name{name}
{
  initWindow();
}

CoreVuWindow::~CoreVuWindow()
{
  glfwDestroyWindow(m_window);
  glfwTerminate();
}

void corevu::CoreVuWindow::CreateWindowSurface(
    VkInstance& ins, VkSurfaceKHR* sur)
{
  if (glfwCreateWindowSurface(ins, m_window, nullptr, sur) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create window surface");
  }
}

void CoreVuWindow::framebufferResizeCallback(
    GLFWwindow* window, int width, int height)
{
  auto corevu_window =
      reinterpret_cast<CoreVuWindow*>(glfwGetWindowUserPointer(window));
  corevu_window->m_framebuffer_is_resized = true;
  corevu_window->m_width = width;
  corevu_window->m_height = height;
}

void CoreVuWindow::initWindow()
{
  if (glfwInit())
  {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(
        m_width, m_height, m_window_name.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(
        m_window, this); // adds user pointer for given window, so that it could
                         // be used in glfw callbacks
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);

    uint32_t extensions_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, nullptr);

    std::cout << "Window is created! Extensions:" << extensions_count
              << std::endl;
  }
}
