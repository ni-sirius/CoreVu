#include <corevu/include/corevu_device.hpp>
#include <corevu/include/corevu_gameobject.hpp>
#include <corevu/include/corevu_window.hpp>
#include "renderer.hpp"

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <memory>

namespace corevutest
{

class SampleApp
{
public:
  static constexpr int width = 800;
  static constexpr int height = 600;

  SampleApp();
  ~SampleApp();
  SampleApp(const SampleApp&) = delete;
  SampleApp& operator=(const SampleApp&) = delete;

  void run();

private:
  void loadGameObjects();

private:
  corevu::CoreVuWindow m_corevu_window{width, height, "hello world!"};
  corevu::CoreVuDevice m_corevu_device{m_corevu_window};
  SampleRenderer m_renderer{m_corevu_window, m_corevu_device};

  std::vector<corevu::CoreVuGameObject>
      m_game_objects; // or // std::unique_ptr<corevu::CoreVuModel>
                      // m_corevu_model;
};
} // namespace corevutest