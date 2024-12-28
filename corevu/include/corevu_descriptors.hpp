#pragma once

#include <corevu_device.hpp>

// std
#include <memory>
#include <unordered_map>
#include <vector>

/** BIG DESCRIPTORS NOTE:
 * USE CASE:
 * . Descriptors are "handles" to resources that the shaders will use.
 * . It makes sense to use separate descriptors for different types of
 *    resources. e.g. buffer data(each data set has their own offsets and
 * sizes), image data(because images specify the layout) . It makes sense to
 * group descriptors in sets depending on how they are used in the shaders. e.g.
 * one set ofr VP matrices and lights(used in each object), one set for
 *    textures(groups of objects), one for Model matrix(individual for each
 * object) . Desctiptor layouts must be defined for each pipeline layout ON
 * CREATION(!) and can't be changed in runtime, which means that different
 * shaders require different pipeline layouts. BUT re-binding pipeline is
 * explensive to objects must be rendered in groups with the same pipeline
 * layout. . Descriptor sets are allocated from descriptor pools, which are
 * created with a maximum number of sets that can be allocated from it.
 */

namespace corevu
{
class CoreVuDescriptorSetLayout
{
public:
  /* For simplifying construction of unordered map of Bindings. */
  class Builder
  {
  public:
    Builder(CoreVuDevice& device) : m_device{device}
    {
    }

    Builder& addBinding(
        uint32_t binding /* unique per layout */,
        VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count = 1 /* each binding could have array of descriptors of that type(useful for instancig?) */);
    std::unique_ptr<CoreVuDescriptorSetLayout> build() const;

  private:
    CoreVuDevice& m_device;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_bindings{};
  };

  /* Builder is used to calc binding here. */
  CoreVuDescriptorSetLayout(
      CoreVuDevice& device,
      std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
  ~CoreVuDescriptorSetLayout();
  CoreVuDescriptorSetLayout(const CoreVuDescriptorSetLayout&) = delete;
  CoreVuDescriptorSetLayout& operator=(const CoreVuDescriptorSetLayout&) =
      delete;

  VkDescriptorSetLayout getDescriptorSetLayout() const
  {
    return m_descriptor_set_layout;
  }

private:
  CoreVuDevice& m_device;
  VkDescriptorSetLayout m_descriptor_set_layout;
  std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_bindings;

  friend class CoreVuDescriptorWriter;
};

class CoreVuDescriptorPool
{
public:
  class Builder
  {
  public:
    Builder(CoreVuDevice& coreVuDevice) : m_device{coreVuDevice}
    {
    }

    Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
    Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
    Builder& setMaxSets(uint32_t count);
    std::unique_ptr<CoreVuDescriptorPool> build() const;

  private:
    CoreVuDevice& m_device;
    std::vector<VkDescriptorPoolSize> m_pool_sizes{};
    uint32_t m_max_sets = 1000;
    VkDescriptorPoolCreateFlags m_pool_flags = 0;
  };

  CoreVuDescriptorPool(
      CoreVuDevice& coreVuDevice, uint32_t maxSets,
      VkDescriptorPoolCreateFlags poolFlags,
      const std::vector<VkDescriptorPoolSize>& poolSizes);
  ~CoreVuDescriptorPool();
  CoreVuDescriptorPool(const CoreVuDescriptorPool&) = delete;
  CoreVuDescriptorPool& operator=(const CoreVuDescriptorPool&) = delete;

  bool allocateDescriptor(
      const VkDescriptorSetLayout descriptorSetLayout,
      VkDescriptorSet& descriptor) const;

  void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

  void resetPool();

private:
  CoreVuDevice& m_device;
  VkDescriptorPool m_descriptor_pool;

  friend class CoreVuDescriptorWriter;
};

/* Class to make creation of descriptors easier. */
class CoreVuDescriptorWriter
{
public:
  CoreVuDescriptorWriter(
      CoreVuDescriptorSetLayout& setLayout, CoreVuDescriptorPool& pool);

  CoreVuDescriptorWriter& writeBuffer(
      uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
  CoreVuDescriptorWriter& writeImage(
      uint32_t binding, VkDescriptorImageInfo* imageInfo);

  bool build(VkDescriptorSet& set);
  void overwrite(VkDescriptorSet& set);

private:
  CoreVuDescriptorSetLayout& m_set_ayout;
  CoreVuDescriptorPool& m_pool;
  std::vector<VkWriteDescriptorSet> m_writes;
};

} // namespace corevu