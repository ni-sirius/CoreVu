#pragma once

#include "corevu_device.hpp"

// libs
#include <vulkan/vulkan.h>

// std
#include <memory>
#include <string>

namespace corevu
{
class CoreVuTexture
{
public:
  CoreVuTexture(CoreVuDevice& device, const std::string& textureFilepath);
  CoreVuTexture(
      CoreVuDevice& device, VkFormat format, VkExtent3D extent,
      VkImageUsageFlags usage, VkSampleCountFlagBits sampleCount);
  ~CoreVuTexture();

  // delete copy constructors
  CoreVuTexture(const CoreVuTexture&) = delete;
  CoreVuTexture& operator=(const CoreVuTexture&) = delete;

  VkImageView imageView() const
  {
    return m_texture_image_view;
  }
  VkSampler sampler() const
  {
    return m_texture_sampler;
  }
  VkImage getImage() const
  {
    return m_texture_image;
  }
  VkImageView getImageView() const
  {
    return m_texture_image_view;
  }
  VkDescriptorImageInfo getImageInfo() const
  {
    return m_descriptor;
  }
  VkImageLayout getImageLayout() const
  {
    return m_texture_layout;
  }
  VkExtent3D getExtent() const
  {
    return m_extent;
  }
  VkFormat getFormat() const
  {
    return m_format;
  }

  void updateDescriptor();
  void transitionLayout(
      VkCommandBuffer commandBuffer, VkImageLayout oldLayout,
      VkImageLayout newLayout);

  static std::shared_ptr<CoreVuTexture> createTextureFromPath(
      CoreVuDevice& device, const std::string& filepath);

private:
  void createTextureImage(const std::string& filepath);
  void createTextureImageView(VkImageViewType viewType);
  void createTextureSampler();

  VkDescriptorImageInfo m_descriptor{};

  CoreVuDevice& m_device;
  VkImage m_texture_image = nullptr;
  VkDeviceMemory m_texture_image_memory = nullptr;
  VkImageView m_texture_image_view = nullptr;
  VkSampler m_texture_sampler = nullptr;
  VkFormat m_format;
  VkImageLayout m_texture_layout;
  uint32_t m_mip_levels{1};
  uint32_t m_layer_count{1};
  VkExtent3D m_extent{};
};

} // namespace corevu