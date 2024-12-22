#include "corevu_model.hpp"

#include <cassert>

#include <Tracy.hpp>

using namespace corevu;

CoreVuModel::CoreVuModel(CoreVuDevice& device, const Builder& builder)
  : m_corevu_device{device}
{
  createVertexBuffers(builder.vertices);
  createIndexBuffers(builder.indices);
}

CoreVuModel::~CoreVuModel()
{
  vkDestroyBuffer(m_corevu_device.device(), m_vertex_buffer, nullptr);
  vkFreeMemory(m_corevu_device.device(), m_buffer_memory, nullptr);

  if (m_had_index_buffer)
  {
    vkDestroyBuffer(m_corevu_device.device(), m_index_buffer, nullptr);
    vkFreeMemory(m_corevu_device.device(), m_index_buffer_memory, nullptr);
  }
}

void CoreVuModel::Draw(VkCommandBuffer command_buffer)
{
  if (m_had_index_buffer)
  {
    vkCmdDrawIndexed(command_buffer, m_index_count, 1, 0, 0, 0);
  }
  else
  {
    vkCmdDraw(command_buffer, m_vertex_count, 1, 0, 0);
  }
}

void CoreVuModel::Bind(VkCommandBuffer command_buffer)
{
  VkBuffer buffers[] = {m_vertex_buffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);

  if (m_had_index_buffer)
  {
    vkCmdBindIndexBuffer(
        command_buffer, m_index_buffer, 0,
        VK_INDEX_TYPE_UINT32); // uint32_t can store up to 2^32 -1(4294967295)
                               // vertices
  }
}

void CoreVuModel::createVertexBuffers(const std::vector<Vertex>& vertices)
{
  ZoneScoped;

  m_vertex_count = static_cast<uint32_t>(vertices.size());
  assert(m_vertex_count >= 3 && "Vertex cound must be at least 3");

  VkDeviceSize buffer_size = sizeof(vertices[0]) * m_vertex_count;
  m_corevu_device.createBuffer(
      buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT /* make buffer memory writable for cpu
                                           */
          | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT /* make cpu buffer mem in sync
                                                    with gpu buffer mem */
      ,
      m_vertex_buffer, m_buffer_memory);

  void* data;
  vkMapMemory(
      m_corevu_device.device(), m_buffer_memory, 0, buffer_size, 0,
      &data); /* creates buffer (data) on cpu size, corresponding with gpu
                 buffer */
  memcpy(data, vertices.data(), static_cast<size_t>(buffer_size));
  vkUnmapMemory(m_corevu_device.device(), m_buffer_memory);

  /* if there was no VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, we would need to call
   * vkFlushMappedMemoryRanges() in order to sync cpu and gpu buffers */
}

void CoreVuModel::createIndexBuffers(const std::vector<Index>& indices)
{
  ZoneScoped;

  m_index_count = static_cast<uint32_t>(indices.size());
  m_had_index_buffer = m_index_count > 0;

  if (!m_had_index_buffer)
  {
    return;
  }

  VkDeviceSize buffer_size = sizeof(indices[0]) * m_index_count;
  m_corevu_device.createBuffer(
      buffer_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      m_index_buffer, m_index_buffer_memory);

  void* data;
  vkMapMemory(
      m_corevu_device.device(), m_index_buffer_memory, 0, buffer_size, 0,
      &data);
  memcpy(data, indices.data(), static_cast<size_t>(buffer_size));
  vkUnmapMemory(m_corevu_device.device(), m_index_buffer_memory);
}

std::vector<VkVertexInputBindingDescription>
CoreVuModel::Vertex::GetBindingDescriptions()
{
  std::vector<VkVertexInputBindingDescription> binding_descriptions(1);
  binding_descriptions[0].binding = 0;
  binding_descriptions[0].stride = sizeof(Vertex);
  binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return binding_descriptions;
}

std::vector<VkVertexInputAttributeDescription>
CoreVuModel::Vertex::GetAttributeDescriptions()
{
  std::vector<VkVertexInputAttributeDescription> attribute_descriptions(2);
  attribute_descriptions[0].binding = 0;
  attribute_descriptions[0].location = 0;
  attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  attribute_descriptions[0].offset = offsetof(Vertex, position); /* 0 */

  attribute_descriptions[1].binding = 0;
  attribute_descriptions[1].location = 1;
  attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  attribute_descriptions[1].offset = offsetof(Vertex, color);

  return attribute_descriptions;
}
