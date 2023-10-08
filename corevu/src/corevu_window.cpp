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

void CoreVuWindow::initWindow()
{
  if (glfwInit())
  {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(
        m_width, m_height, m_window_name.c_str(), nullptr, nullptr);

    uint32_t extensions_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, nullptr);

    std::cout << "Window is created! Extensions:" << extensions_count << std::endl;
  }
}
