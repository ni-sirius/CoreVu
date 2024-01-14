#include <corevu/include/corevu_device.hpp>
#include <corevu/include/corevu_gameobject.hpp>
#include <corevu/include/corevu_window.hpp>
#include <corevu/include/corevu_pipeline.hpp>

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <memory>

namespace corevutest
{

class RenderSystem
{
public:
  RenderSystem(corevu::CoreVuDevice& device, VkRenderPass render_pass);
  ~RenderSystem();
  RenderSystem(const RenderSystem&) = delete;
  RenderSystem& operator=(const RenderSystem&) = delete;

  void renderGameObjects(
      VkCommandBuffer command_buffer,
      std::vector<corevu::CoreVuGameObject>& game_objects);

private:
  void createPipelineLayout();
  void createPipeline(VkRenderPass render_pass);

private:
  corevu::CoreVuDevice& m_corevu_device;

  std::unique_ptr<corevu::CoreVuPipeline> m_corevu_pipeline{nullptr};
  VkPipelineLayout m_pipeline_layout;
};
} // namespace corevutest