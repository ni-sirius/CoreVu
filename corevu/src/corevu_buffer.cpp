/*
 * Encapsulates a vulkan buffer
 *
 * Initially based off VulkanBuffer by Sascha Willems -
 * https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
 */

#include <corevu_buffer.hpp>

// std
#include <cassert>
#include <cstring>

using namespace corevu;

/**
 * Returns the minimum instance size required to be compatible with devices minOffsetAlignment
 *
 * @param instanceSize The size of an instance
 * @param minOffsetAlignment The minimum required alignment, in bytes, for the offset member (eg
 * minUniformBufferOffsetAlignment)
 *
 * @return VkResult of the buffer mapping call
 */
VkDeviceSize CoreVuBuffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
  if (minOffsetAlignment > 0) {
    return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
  }
  return instanceSize;
}
 
CoreVuBuffer::CoreVuBuffer(
    CoreVuDevice &device,
    VkDeviceSize instanceSize,
    uint32_t instanceCount,
    VkBufferUsageFlags usageFlags,
    VkMemoryPropertyFlags memoryPropertyFlags,
    VkDeviceSize minOffsetAlignment)
    : m_device{device},
      m_instance_size{instanceSize},
      m_instance_count{instanceCount},
      m_usage_flags{usageFlags},
      m_memory_property_flags{memoryPropertyFlags} {
  m_alignment_size = getAlignment(instanceSize, minOffsetAlignment);
  m_buffer_size = m_alignment_size * instanceCount;
  device.createBuffer(m_buffer_size, usageFlags, memoryPropertyFlags, m_buffer, m_memory);
}
 
CoreVuBuffer::~CoreVuBuffer() {
  unmap();
  vkDestroyBuffer(m_device.device(), m_buffer, nullptr);
  vkFreeMemory(m_device.device(), m_memory, nullptr);
}
 
/**
 * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
 *
 * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete
 * buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the buffer mapping call
 */
VkResult CoreVuBuffer::map(VkDeviceSize size, VkDeviceSize offset) {
  assert(m_buffer && m_memory && "Called map on buffer before create");
  return vkMapMemory(m_device.device(), m_memory, offset, size, 0, &m_mapped);
}
 
/**
 * Unmap a mapped memory range
 *
 * @note Does not return a result as vkUnmapMemory can't fail
 */
void CoreVuBuffer::unmap() {
  if (m_mapped) {
    vkUnmapMemory(m_device.device(), m_memory);
    m_mapped = nullptr;
  }
}
 
/**
 * Copies the specified data to the mapped buffer. Default value writes whole buffer range
 *
 * @param data Pointer to the data to copy
 * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer
 * range.
 * @param offset (Optional) Byte offset from beginning of mapped region
 *
 */
void CoreVuBuffer::writeToBuffer(void *data, VkDeviceSize size, VkDeviceSize offset) {
  assert(m_mapped && "Cannot copy to unmapped buffer");
 
  if (size == VK_WHOLE_SIZE) {
    memcpy(m_mapped, data, m_buffer_size);
  } else {
    char *memOffset = (char *)m_mapped;
    memOffset += offset;
    memcpy(memOffset, data, size);
  }
}
 
/**
 * Flush a memory range of the buffer to make it visible to the device
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the
 * complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the flush call
 */
VkResult CoreVuBuffer::flush(VkDeviceSize size, VkDeviceSize offset) {
  VkMappedMemoryRange mappedRange = {};
  mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  mappedRange.memory = m_memory;
  mappedRange.offset = offset;
  mappedRange.size = size;
  return vkFlushMappedMemoryRanges(m_device.device(), 1, &mappedRange);
}
 
/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate
 * the complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the invalidate call
 */
VkResult CoreVuBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
  VkMappedMemoryRange mappedRange = {};
  mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  mappedRange.memory = m_memory;
  mappedRange.offset = offset;
  mappedRange.size = size;
  return vkInvalidateMappedMemoryRanges(m_device.device(), 1, &mappedRange);
}
 
/**
 * Create a buffer info descriptor
 *
 * @param size (Optional) Size of the memory range of the descriptor
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkDescriptorBufferInfo of specified offset and range
 */
VkDescriptorBufferInfo CoreVuBuffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
  return VkDescriptorBufferInfo{
      m_buffer,
      offset,
      size,
  };
}
 
/**
 * Copies "instanceSize" bytes of data to the mapped buffer at an offset of index * alignmentSize
 *
 * @param data Pointer to the data to copy
 * @param index Used in offset calculation
 *
 */
void CoreVuBuffer::writeToIndex(void *data, int index) {
  writeToBuffer(data, m_instance_size, index * m_alignment_size);
}
 
/**
 *  Flush the memory range at index * alignmentSize of the buffer to make it visible to the device
 *
 * @param index Used in offset calculation
 *
 */
VkResult CoreVuBuffer::flushIndex(int index) { return flush(m_alignment_size, index * m_alignment_size); }
 
/**
 * Create a buffer info descriptor
 *
 * @param index Specifies the region given by index * alignmentSize
 *
 * @return VkDescriptorBufferInfo for instance at index
 */
VkDescriptorBufferInfo CoreVuBuffer::descriptorInfoForIndex(int index) {
  return descriptorInfo(m_alignment_size, index * m_alignment_size);
}
 
/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 *
 * @param index Specifies the region to invalidate: index * alignmentSize
 *
 * @return VkResult of the invalidate call
 */
VkResult CoreVuBuffer::invalidateIndex(int index) {
  return invalidate(m_alignment_size, index * m_alignment_size);
}