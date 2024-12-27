#pragma once

#include <corevu_device.hpp>

namespace corevu
{

/* NOTE:
  for uniform bufers it's required to have an alignment, which is realtive to
  each machine. e.g. 16bytes. So the offset/padding must be added in back of the
  structure.
  There is no such requirement for index and vertex buffers == 1*/
class CoreVuBuffer
{
public:
  CoreVuBuffer(
      CoreVuDevice& device, VkDeviceSize instanceSize, uint32_t instanceCount,
      VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags,
      VkDeviceSize minOffsetAlignment = 1);
  ~CoreVuBuffer();

  CoreVuBuffer(const CoreVuBuffer&) = delete;
  CoreVuBuffer& operator=(const CoreVuBuffer&) = delete;

  VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  void unmap();

  void writeToBuffer(
      void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  VkDescriptorBufferInfo descriptorInfo(
      VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  VkResult invalidate(
      VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

  void writeToIndex(void* data, int index);
  VkResult flushIndex(int index);
  VkDescriptorBufferInfo descriptorInfoForIndex(int index);
  VkResult invalidateIndex(int index);

  VkBuffer getBuffer() const
  {
    return m_buffer;
  }
  void* getMappedMemory() const
  {
    return m_mapped;
  }
  uint32_t getInstanceCount() const
  {
    return m_instance_count;
  }
  VkDeviceSize getInstanceSize() const
  {
    return m_instance_size;
  }
  VkDeviceSize getAlignmentSize() const
  {
    return m_instance_size;
  }
  VkBufferUsageFlags getUsageFlags() const
  {
    return m_usage_flags;
  }
  VkMemoryPropertyFlags getMemoryPropertyFlags() const
  {
    return m_memory_property_flags;
  }
  VkDeviceSize getBufferSize() const
  {
    return m_buffer_size;
  }

private:
  static VkDeviceSize getAlignment(
      VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

  CoreVuDevice& m_device;
  void* m_mapped = nullptr;
  VkBuffer m_buffer = VK_NULL_HANDLE;
  VkDeviceMemory m_memory = VK_NULL_HANDLE;

  VkDeviceSize m_buffer_size;
  uint32_t m_instance_count;
  VkDeviceSize m_instance_size;
  VkDeviceSize m_alignment_size;
  VkBufferUsageFlags m_usage_flags;
  VkMemoryPropertyFlags m_memory_property_flags;
};

} // namespace corevu
