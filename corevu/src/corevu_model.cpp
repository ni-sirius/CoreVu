#include "corevu_model.hpp"
#include <global_utils.hpp>

// libs
#include <Tracy.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// std
#include <cassert>
#include <unordered_map>
#include <iostream>

namespace std
{
template <>
struct hash<corevu::CoreVuModel::Vertex>
{
  size_t operator()(corevu::CoreVuModel::Vertex const& vertex) const
  {
    size_t seed = 0;
    corevu::hashCombine(
        seed, vertex.position, vertex.color, vertex.normal, vertex.texCoord);
    return seed;
  }
};
} // namespace std

using namespace corevu;

CoreVuModel::CoreVuModel(CoreVuDevice& device, const Builder& builder)
  : m_corevu_device{device}
{
  createVertexBuffers(builder.vertices);
  createIndexBuffers(builder.indices);
}

CoreVuModel::~CoreVuModel()
{
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

std::shared_ptr<CoreVuModel> corevu::CoreVuModel::CreateModelFromPath(
    CoreVuDevice& device, const std::string& path)
{
  Builder builder{};
  builder.loadModel(path);
  std::cout << "Load Model:" << path
            << " vertex count:" << builder.vertices.size() << std::endl;

  return std::make_shared<CoreVuModel>(device, builder);
}

void CoreVuModel::Bind(VkCommandBuffer command_buffer)
{
  if (m_vertex_buffer)
  {
    VkBuffer buffers[] = {m_vertex_buffer->getBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);
  }

  if (m_had_index_buffer && m_index_buffer)
  {
    vkCmdBindIndexBuffer(
        command_buffer, m_index_buffer->getBuffer(), 0,
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

  /* Old, manual buffer creation, just for info purpose. The usage you can find by this commit. */
  //   /* Use more optimized local device mem on GPU side using
  //   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT. The momeory is accessible only from
  //   gpu, that's why we creating staging buffer and then copy it's contents to
  //   GPU Local buffer. It makes sense to use only host memory for less
  //   frequently updated data, like textures, because it's slower to access
  //   from gpu.

  //   Staging buffers for now are optimal only for static data uploaded at the
  //   start of the app.

  //   the m_corevu_device.copyBuffer() is not optimal though, because it uses
  //   single time command, could be more efficint to use some memory barrier
  //   stuff here.*/ VkBuffer staging_buffer; VkDeviceMemory
  //   staging_buffer_memory; m_corevu_device.createBuffer(
  //       buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, /* make buffer as
  //       source
  //                                                         for memory
  //                                                         operation */
  //       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT /* make buffer memory writable
  //       for cpu
  //                                            */
  //           | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT /* make cpu buffer mem in
  //           sync
  //                                                     with gpu buffer mem */
  //       ,
  //       staging_buffer, staging_buffer_memory);

  //   void* data;
  //   vkMapMemory(
  //       m_corevu_device.device(), staging_buffer_memory, 0, buffer_size, 0,
  //       &data); /* creates buffer (data) on cpu size, corresponding with gpu
  //                  buffer */
  //   memcpy(data, vertices.data(), static_cast<size_t>(buffer_size));
  //   vkUnmapMemory(m_corevu_device.device(), staging_buffer_memory);

  //   m_corevu_device.createBuffer(
  //       buffer_size,
  //       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT /* use buffer as vertex buf */ |
  //           VK_BUFFER_USAGE_TRANSFER_DST_BIT, /* make buffer as destination
  // for memory operation */
  //       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT /* make buffer memory writable
  //       for gpu
  //                                              with most efficent speed*/
  //       ,
  //       m_vertex_buffer, m_buffer_memory);

  //   m_corevu_device.copyBuffer(
  //       staging_buffer, m_vertex_buffer,
  //       buffer_size); // to be optimized with memory barrier

  //   vkDestroyBuffer(m_corevu_device.device(), staging_buffer, nullptr);
  //   vkFreeMemory(m_corevu_device.device(), staging_buffer_memory, nullptr);

  //   /* if there was no VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, we would need to
  //   call
  //    * vkFlushMappedMemoryRanges() in order to sync cpu and gpu buffers */

  uint32_t vertex_size = sizeof(vertices[0]);
  CoreVuBuffer staging_buffer{
      m_corevu_device, vertex_size, m_vertex_count,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

  staging_buffer.map();
  staging_buffer.writeToBuffer((void*)vertices.data());

  m_vertex_buffer = std::make_unique<CoreVuBuffer>(
      m_corevu_device, vertex_size, m_vertex_count,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  m_corevu_device.copyBuffer(
      staging_buffer.getBuffer(), m_vertex_buffer->getBuffer(), buffer_size);
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

  uint32_t index_size = sizeof(indices[0]);
  CoreVuBuffer staging_buffer{
      m_corevu_device, index_size, m_index_count,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

  staging_buffer.map();
  staging_buffer.writeToBuffer((void*)indices.data());

  m_index_buffer = std::make_unique<CoreVuBuffer>(
      m_corevu_device, index_size, m_index_count,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  m_corevu_device.copyBuffer(
      staging_buffer.getBuffer(), m_index_buffer->getBuffer(), buffer_size);
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
  std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};
  {
    VkVertexInputAttributeDescription attribute_description{};
    attribute_description.location = 0; // location in shader
    attribute_description.binding = 0;  // binding in shader
    attribute_description.format = VK_FORMAT_R32G32B32_SFLOAT; // 3 floats
    attribute_description.offset = offsetof(Vertex, position); // offset in
                                                               // struct
    attribute_descriptions.push_back(std::move(attribute_description));
  }
  {
    VkVertexInputAttributeDescription attribute_description{};
    attribute_description.location = 1;
    attribute_description.binding = 0;
    attribute_description.format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_description.offset = offsetof(Vertex, color);
    attribute_descriptions.push_back(std::move(attribute_description));
  }
  {
    VkVertexInputAttributeDescription attribute_description{};
    attribute_description.location = 2;
    attribute_description.binding = 0;
    attribute_description.format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_description.offset = offsetof(Vertex, normal);
    attribute_descriptions.push_back(std::move(attribute_description));
  }
  {
    VkVertexInputAttributeDescription attribute_description{};
    attribute_description.location = 3;
    attribute_description.binding = 0;
    attribute_description.format = VK_FORMAT_R32G32_SFLOAT;
    attribute_description.offset = offsetof(Vertex, texCoord);
    attribute_descriptions.push_back(std::move(attribute_description));
  }

  return attribute_descriptions;
}

// Builder methods implementation //

void corevu::CoreVuModel::Builder::loadModel(const std::string& filename)
{
  ZoneScoped;

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  if (!tinyobj::LoadObj(
          &attrib, &shapes, &materials, &warn, &err, filename.c_str()))
  {
    throw std::runtime_error(warn + err);
  }

  vertices.clear();
  indices.clear();

  std::unordered_map<Vertex, Index> unique_vertices{};
  for (const auto& shape : shapes)
  {
    for (const auto& index : shape.mesh.indices)
    {
      Vertex vertex{};

      if (index.vertex_index >= 0)
      {
        vertex.position = {
            attrib.vertices[3 * index.vertex_index + 0],
            attrib.vertices[3 * index.vertex_index + 1],
            attrib.vertices[3 * index.vertex_index + 2]};

        vertex.color = {
            attrib.colors[3 * index.vertex_index + 0],
            attrib.colors[3 * index.vertex_index + 1],
            attrib.colors[3 * index.vertex_index + 2]};
      }

      if (index.normal_index >= 0)
      {
        vertex.normal = {
            attrib.normals[3 * index.normal_index + 0],
            attrib.normals[3 * index.normal_index + 1],
            attrib.normals[3 * index.normal_index + 2]};
      }

      if (index.texcoord_index >= 0)
      {
        vertex.texCoord = {
            attrib.texcoords[2 * index.texcoord_index + 0],
            1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
      }

      if (unique_vertices.count(vertex) == 0)
      {
        unique_vertices[vertex] = Index{static_cast<uint32_t>(vertices.size())};
        vertices.push_back(vertex);
      }

      indices.push_back(unique_vertices[vertex]);
    }
  }
}
