#include <corevu_descriptors.hpp>

// std
#include <cassert>
#include <stdexcept>

using namespace corevu;

// *************** Descriptor Set Layout Builder *********************

CoreVuDescriptorSetLayout::Builder&
CoreVuDescriptorSetLayout::Builder::addBinding(
    uint32_t binding, VkDescriptorType descriptorType,
    VkShaderStageFlags stageFlags, uint32_t count)
{
  assert(m_bindings.count(binding) == 0 && "Binding already in use");
  VkDescriptorSetLayoutBinding layoutBinding{};
  layoutBinding.binding = binding;
  layoutBinding.descriptorType = descriptorType;
  layoutBinding.descriptorCount = count;
  layoutBinding.stageFlags = stageFlags;
  m_bindings[binding] = layoutBinding;
  return *this;
}

std::unique_ptr<CoreVuDescriptorSetLayout>
CoreVuDescriptorSetLayout::Builder::build() const
{
  return std::make_unique<CoreVuDescriptorSetLayout>(m_device, m_bindings);
}

// *************** Descriptor Set Layout *********************

CoreVuDescriptorSetLayout::CoreVuDescriptorSetLayout(
    CoreVuDevice& device,
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
  : m_device{device}, m_bindings{bindings}
{
  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
  for (auto kv : bindings)
  {
    setLayoutBindings.push_back(kv.second);
  }

  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
  descriptorSetLayoutInfo.sType =
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptorSetLayoutInfo.bindingCount =
      static_cast<uint32_t>(setLayoutBindings.size());
  descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

  if (vkCreateDescriptorSetLayout(
          m_device.device(), &descriptorSetLayoutInfo, nullptr,
          &m_descriptor_set_layout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create descriptor set layout!");
  }
}

CoreVuDescriptorSetLayout::~CoreVuDescriptorSetLayout()
{
  vkDestroyDescriptorSetLayout(
      m_device.device(), m_descriptor_set_layout, nullptr);
}

// *************** Descriptor Pool Builder *********************

CoreVuDescriptorPool::Builder& CoreVuDescriptorPool::Builder::addPoolSize(
    VkDescriptorType descriptorType, uint32_t count)
{
  m_pool_sizes.push_back({descriptorType, count});
  return *this;
}

CoreVuDescriptorPool::Builder& CoreVuDescriptorPool::Builder::setPoolFlags(
    VkDescriptorPoolCreateFlags flags)
{
  m_pool_flags = flags;
  return *this;
}
CoreVuDescriptorPool::Builder& CoreVuDescriptorPool::Builder::setMaxSets(
    uint32_t count)
{
  m_max_sets = count;
  return *this;
}

std::unique_ptr<CoreVuDescriptorPool> CoreVuDescriptorPool::Builder::build()
    const
{
  return std::make_unique<CoreVuDescriptorPool>(
      m_device, m_max_sets, m_pool_flags, m_pool_sizes);
}

// *************** Descriptor Pool *********************

CoreVuDescriptorPool::CoreVuDescriptorPool(
    CoreVuDevice& coreVuDevice, uint32_t maxSets,
    VkDescriptorPoolCreateFlags poolFlags,
    const std::vector<VkDescriptorPoolSize>& poolSizes)
  : m_device{coreVuDevice}
{
  VkDescriptorPoolCreateInfo descriptorPoolInfo{};
  descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  descriptorPoolInfo.pPoolSizes = poolSizes.data();
  descriptorPoolInfo.maxSets = maxSets;
  descriptorPoolInfo.flags = poolFlags;

  if (vkCreateDescriptorPool(
          coreVuDevice.device(), &descriptorPoolInfo, nullptr,
          &m_descriptor_pool) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}

CoreVuDescriptorPool::~CoreVuDescriptorPool()
{
  vkDestroyDescriptorPool(m_device.device(), m_descriptor_pool, nullptr);
}

bool CoreVuDescriptorPool::allocateDescriptor(
    const VkDescriptorSetLayout descriptorSetLayout,
    VkDescriptorSet& descriptor) const
{
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = m_descriptor_pool;
  allocInfo.pSetLayouts = &descriptorSetLayout;
  allocInfo.descriptorSetCount = 1;

  // Might want to create a "DescriptorPoolManager" class that handles this
  // case, and builds a new pool whenever an old pool fills up. But this is
  // beyond our current scope
  // like https://vkguide.dev/docs/extra-chapter/abstracting_descriptors/
  if (vkAllocateDescriptorSets(m_device.device(), &allocInfo, &descriptor) !=
      VK_SUCCESS)
  {
    return false;
  }
  return true;
}

void CoreVuDescriptorPool::freeDescriptors(
    std::vector<VkDescriptorSet>& descriptors) const
{
  vkFreeDescriptorSets(
      m_device.device(), m_descriptor_pool,
      static_cast<uint32_t>(descriptors.size()), descriptors.data());
}

void CoreVuDescriptorPool::resetPool()
{
  vkResetDescriptorPool(m_device.device(), m_descriptor_pool, 0);
}

// *************** Descriptor Writer *********************

CoreVuDescriptorWriter::CoreVuDescriptorWriter(
    CoreVuDescriptorSetLayout& setLayout, CoreVuDescriptorPool& pool)
  : m_set_ayout{setLayout}, m_pool{pool}
{
}

CoreVuDescriptorWriter& CoreVuDescriptorWriter::writeBuffer(
    uint32_t binding, VkDescriptorBufferInfo* bufferInfo)
{
  // Fills m_writes with info about new resource aka adds to list
  assert(
      m_set_ayout.m_bindings.count(binding) == 1 &&
      "Layout does not contain specified binding");

  auto& bindingDescription = m_set_ayout.m_bindings[binding];

  assert(
      bindingDescription.descriptorCount == 1 &&
      "Binding single descriptor info, but binding expects multiple");

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = bindingDescription.descriptorType;
  write.dstBinding = binding;
  write.pBufferInfo = bufferInfo;
  write.descriptorCount = 1;

  m_writes.push_back(write);
  return *this;
}

CoreVuDescriptorWriter& CoreVuDescriptorWriter::writeImage(
    uint32_t binding, VkDescriptorImageInfo* imageInfo)
{
  assert(
      m_set_ayout.m_bindings.count(binding) == 1 &&
      "Layout does not contain specified binding");

  auto& bindingDescription = m_set_ayout.m_bindings[binding];

  assert(
      bindingDescription.descriptorCount == 1 &&
      "Binding single descriptor info, but binding expects multiple");

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = bindingDescription.descriptorType;
  write.dstBinding = binding;
  write.pImageInfo = imageInfo;
  write.descriptorCount = 1;

  m_writes.push_back(write);
  return *this;
}

bool CoreVuDescriptorWriter::build(VkDescriptorSet& set)
{
  // alocates descriptor set and becomes a handle to it(set variable)
  bool success =
      m_pool.allocateDescriptor(m_set_ayout.getDescriptorSetLayout(), set);
  if (!success)
  {
    return false;
  }
  // fills the allocades descriptor set with the resources
  overwrite(set);
  return true;
}

void CoreVuDescriptorWriter::overwrite(VkDescriptorSet& set)
{
  for (auto& write : m_writes)
  {
    write.dstSet = set;
  }
  //sets to write to the newly created set/or alreaady used one
  vkUpdateDescriptorSets(
      m_pool.m_device.device(), static_cast<uint32_t>(m_writes.size()),
      m_writes.data(), 0, nullptr);
}