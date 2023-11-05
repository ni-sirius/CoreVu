#pragma once

#include <string>
#include <vector>

#include <corevu_device.hpp>

namespace corevu
{
struct PipelineConfigInfo
{
  //tt
};

class CoreVuPipeline
{
public:
  CoreVuPipeline(
      CoreVuDevice& device, const PipelineConfigInfo& config_info,
      const std::string& vert_filepath, const std::string& frag_filepath);
  ~CoreVuPipeline() = default;
  CoreVuPipeline(const CoreVuPipeline&) = delete;
  void operator=(const CoreVuPipeline&) = delete;

  static PipelineConfigInfo DefaultPipelineConfigInfo(
      uint32_t width, uint32_t height);

private:
  static std::vector<char> readFile(const std::string& filepath);
  void createGraphicsPipeline(
      const std::string& vert_filepath, const std::string& frag_filepath,
      const PipelineConfigInfo& config_info);

  void createShaderModule(
      const std::vector<char>& code, VkShaderModule* shader_module);

  CoreVuDevice& m_device;
  VkPipeline m_vulkan_pipeline;
  VkShaderModule m_vulkan_vert_shader_module;
  VkShaderModule m_vulkan_frag_shader_module;
};
} // namespace corevu