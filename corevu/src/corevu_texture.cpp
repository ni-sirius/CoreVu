#include <corevu_texture.hpp>

// libs
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// std
#include <cmath>
#include <stdexcept>
#include <iostream>

namespace corevu
{
CoreVuTexture::CoreVuTexture(
    CoreVuDevice& device, const std::string& textureFilepath)
  : m_device{device}
{
  createTextureImage(textureFilepath);
  createTextureImageView(VK_IMAGE_VIEW_TYPE_2D);
  createTextureSampler();
  updateDescriptor();
}

CoreVuTexture::CoreVuTexture(
    CoreVuDevice& device, VkFormat format, VkExtent3D extent,
    VkImageUsageFlags usage, VkSampleCountFlagBits sampleCount)
  : m_device{device}
{
  VkImageAspectFlags aspectMask = 0;
  VkImageLayout imageLayout;

  m_format = format;
  m_extent = extent;

  if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
  {
    aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  }
  if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
  {
    aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  }

  // Don't like this, should I be using an image array instead of multiple
  // images?
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.format = format;
  imageInfo.extent = extent;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.samples = sampleCount;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.usage = usage;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  device.createImageWithInfo(
      imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_texture_image,
      m_texture_image_memory);

  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange = {};
  viewInfo.subresourceRange.aspectMask = aspectMask;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;
  viewInfo.image = m_texture_image;
  if (vkCreateImageView(
          device.device(), &viewInfo, nullptr, &m_texture_image_view) !=
      VK_SUCCESS)
  {
    throw std::runtime_error("failed to create texture image view!");
  }

  // Sampler should be seperated out
  if (usage & VK_IMAGE_USAGE_SAMPLED_BIT)
  {
    // Create sampler to sample from the attachment in the fragment shader
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeV = samplerInfo.addressModeU;
    samplerInfo.addressModeW = samplerInfo.addressModeU;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

    if (vkCreateSampler(
            device.device(), &samplerInfo, nullptr, &m_texture_sampler) !=
        VK_SUCCESS)
    {
      throw std::runtime_error("failed to create sampler!");
    }

    VkImageLayout samplerImageLayout =
        imageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    m_descriptor.sampler = m_texture_sampler;
    m_descriptor.imageView = m_texture_image_view;
    m_descriptor.imageLayout = samplerImageLayout;
  }
}

CoreVuTexture::~CoreVuTexture()
{
  vkDestroySampler(m_device.device(), m_texture_sampler, nullptr);
  vkDestroyImageView(m_device.device(), m_texture_image_view, nullptr);
  vkDestroyImage(m_device.device(), m_texture_image, nullptr);
  vkFreeMemory(m_device.device(), m_texture_image_memory, nullptr);
}

std::shared_ptr<CoreVuTexture> CoreVuTexture::createTextureFromPath(
    CoreVuDevice& device, const std::string& filepath)
{
  return std::make_unique<CoreVuTexture>(device, filepath);
}

void CoreVuTexture::updateDescriptor()
{
  m_descriptor.sampler = m_texture_sampler;
  m_descriptor.imageView = m_texture_image_view;
  m_descriptor.imageLayout = m_texture_layout;
}

void CoreVuTexture::createTextureImage(const std::string& filepath)
{
  int texWidth, texHeight, texChannels;
  // stbi_set_flip_vertically_on_load(1);  // todo determine why texture
  // coordinates are flipped
  stbi_uc* pixels = stbi_load(
      filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  if (!pixels)
  {
    throw std::runtime_error("failed to load texture image!");
  }
  else
  {
    std::cout << "Loaded texture image: " << filepath << " w:" << texWidth
              << " h:" << texHeight << " chn:" << texChannels << std::endl;
  }

  // mMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth,
  // texHeight)))) + 1;
  m_mip_levels = 1;

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  m_device.createBuffer(
      imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      stagingBuffer, stagingBufferMemory);

  void* data;
  vkMapMemory(m_device.device(), stagingBufferMemory, 0, imageSize, 0, &data);
  memcpy(data, pixels, static_cast<size_t>(imageSize));
  vkUnmapMemory(m_device.device(), stagingBufferMemory);

  stbi_image_free(pixels);

  m_format = VK_FORMAT_R8G8B8A8_SRGB;
  m_extent = {
      static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1};

  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent = m_extent;
  imageInfo.mipLevels = m_mip_levels;
  imageInfo.arrayLayers = m_layer_count;
  imageInfo.format = m_format;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                    VK_IMAGE_USAGE_SAMPLED_BIT;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  m_device.createImageWithInfo(
      imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_texture_image,
      m_texture_image_memory);
  m_device.transitionImageLayout(
      m_texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_mip_levels, m_layer_count);
  m_device.copyBufferToImage(
      stagingBuffer, m_texture_image, static_cast<uint32_t>(texWidth),
      static_cast<uint32_t>(texHeight), m_layer_count);

  // comment this out if using mips
  m_device.transitionImageLayout(
      m_texture_image, VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_mip_levels, m_layer_count);

  // If we generate mip maps then the final image will alerady be
  // READ_ONLY_OPTIMAL mDevice.generateMipmaps(mTextureImage, mFormat, texWidth,
  // texHeight, mMipLevels);
  m_texture_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  vkDestroyBuffer(m_device.device(), stagingBuffer, nullptr);
  vkFreeMemory(m_device.device(), stagingBufferMemory, nullptr);
}

void CoreVuTexture::createTextureImageView(VkImageViewType viewType)
{
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = m_texture_image;
  viewInfo.viewType = viewType;
  viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = m_mip_levels;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = m_layer_count;

  if (vkCreateImageView(
          m_device.device(), &viewInfo, nullptr, &m_texture_image_view) !=
      VK_SUCCESS)
  {
    throw std::runtime_error("failed to create texture image view!");
  }
}

void CoreVuTexture::createTextureSampler()
{
  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;

  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

  samplerInfo.anisotropyEnable = VK_TRUE;
  samplerInfo.maxAnisotropy = 16.0f;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;

  // this fields useful for percentage close filtering for shadow maps
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = static_cast<float>(m_mip_levels);

  if (vkCreateSampler(
          m_device.device(), &samplerInfo, nullptr, &m_texture_sampler) !=
      VK_SUCCESS)
  {
    throw std::runtime_error("failed to create texture sampler!");
  }
}

void CoreVuTexture::transitionLayout(
    VkCommandBuffer commandBuffer, VkImageLayout oldLayout,
    VkImageLayout newLayout)
{
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;

  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

  barrier.image = m_texture_image;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = m_mip_levels;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = m_layer_count;

  if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
  {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (m_format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
        m_format == VK_FORMAT_D24_UNORM_S8_UINT)
    {
      barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
  }
  else
  {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
      newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
  {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else if (
      oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
      newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
  {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else if (
      oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
      newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
  {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }
  else if (
      oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
      newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
  {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  }
  else if (
      oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
      newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
  {
    // This says that any cmd that acts in color output or after (dstStage)
    // that needs read or write access to a resource
    // must wait until all previous read accesses in fragment shader
    barrier.srcAccessMask =
        VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  }
  else
  {
    throw std::invalid_argument("unsupported layout transition!");
  }
  vkCmdPipelineBarrier(
      commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr,
      1, &barrier);
}
} // namespace corevu