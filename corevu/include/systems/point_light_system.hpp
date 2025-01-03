#pragma once

#include "corevu_camera.hpp"
#include "corevu_device.hpp"
#include "corevu_gameobject.hpp"
#include "corevu_window.hpp"
#include "corevu_pipeline.hpp"
#include "corevu_frame_info.hpp"

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <memory>

namespace corevu
{

/* The whole system is TEMPORARY and used only for diplaying HARDCODED set of
   graphic primitives and sets from app.hpp/cpp. It's not generic at all and
   must be used CAREFULLY!. Won't be used in the final version.
 */

class PointLightSystem
{
public:
  PointLightSystem(
      CoreVuDevice& device, VkRenderPass render_pass,
      VkDescriptorSetLayout global_descriptor_set_layout);
  ~PointLightSystem();
  PointLightSystem(const PointLightSystem&) = delete;
  PointLightSystem& operator=(const PointLightSystem&) = delete;

  void update(FrameInfo& frame_info, GlobalUbo& global_ubo);
  void render(FrameInfo& frame_info);

private:
  void createPipelineLayout(VkDescriptorSetLayout global_descriptor_set_layout);
  void createPipeline(VkRenderPass render_pass);

private:
  CoreVuDevice& m_corevu_device;

  std::unique_ptr<CoreVuPipeline> m_corevu_pipeline{nullptr};
  VkPipelineLayout m_pipeline_layout;
};
} // namespace corevu