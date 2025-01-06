#pragma once

#include <corevu_camera.hpp>
#include <corevu_descriptors.hpp>
#include <corevu_device.hpp>
#include <corevu_frame_info.hpp>
#include <corevu_gameobject.hpp>
#include <corevu_pipeline.hpp>

// std
#include <memory>
#include <vector>

namespace corevu
{
class TextureRenderSystem
{
public:
  TextureRenderSystem(
      CoreVuDevice& device, VkRenderPass renderPass,
      VkDescriptorSetLayout globalSetLayout);
  ~TextureRenderSystem();

  TextureRenderSystem(const TextureRenderSystem&) = delete;
  TextureRenderSystem& operator=(const TextureRenderSystem&) = delete;

  void renderGameObjects(FrameInfo& frameInfo);

private:
  void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
  void createPipeline(VkRenderPass renderPass);

  CoreVuDevice& m_device;

  std::unique_ptr<CoreVuPipeline> m_pipeline;
  VkPipelineLayout m_pipeline_layout;

  std::unique_ptr<CoreVuDescriptorSetLayout> m_render_system_layout;
};
} // namespace corevu
