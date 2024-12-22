#pragma once

#include "corevu_device.hpp"

#define GLM_FORCE_RADIANS           // to be sure that no change depending on system
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // instead of -1 to 1 ?
#include <glm/glm.hpp>

#include <vector>

namespace corevu
{
class CoreVuModel
{
public:
  /* NOTE:
  don't forget to update attribute description(GetAttributeDescriptions()) in
  vulkan every time you change the Vertex struct. */
  struct Vertex
  {
    glm::vec3 position;
    glm::vec3 color;

    static std::vector<VkVertexInputBindingDescription>
    GetBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription>
    GetAttributeDescriptions();
  };

  struct Index
  {
    uint32_t value;

    Index(uint32_t v) : value(v)
    {
    }
  };

  struct Builder
  {
    std::vector<Vertex> vertices{};
    std::vector<Index> indices{};
  };

  CoreVuModel(CoreVuDevice& device, const Builder& builder);
  ~CoreVuModel();

  CoreVuModel(const CoreVuModel&) = delete;
  CoreVuModel& operator=(const CoreVuModel&) = delete;

  void Bind(VkCommandBuffer command_buffer);
  void Draw(VkCommandBuffer command_buffer);

private:
  void createVertexBuffers(const std::vector<Vertex>& vertices);
  void createIndexBuffers(const std::vector<Index>& indices);

private:
  CoreVuDevice& m_corevu_device;

  /* Buffer and memory are presented as different objects, because allocation of
   * memory takes time and there is limit of buffers in vk device, so it's
   * recommended to allocate big chunks of memory at one and use it between
   * buffers. Sprecial lib for memory allocation is Vulkan Memory Allocator lib.
   * Self-written solution -
   * http://kylehalladay.com/blog/tutorial/2017/12/13/Custom-Allocators-Vulkan.html
   */
  VkBuffer m_vertex_buffer;
  VkDeviceMemory m_buffer_memory;
  uint32_t m_vertex_count;

  bool m_had_index_buffer;
  VkBuffer m_index_buffer;
  VkDeviceMemory m_index_buffer_memory;
  uint32_t m_index_count;
};
} // namespace corevu