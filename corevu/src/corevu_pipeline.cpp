#include <corevu_pipeline.hpp>

#include <fstream>
#include <stdexcept>

#include <iostream>

using namespace corevu;

CoreVuPipeline::CoreVuPipeline(
    CoreVuDevice& device, const PipelineConfigInfo& config_info,
    const std::string& vert_filepath, const std::string& frag_filepath)
    : m_device{device}
{
  createGraphicsPipeline(vert_filepath, frag_filepath, config_info);
}

PipelineConfigInfo CoreVuPipeline::DefaultPipelineConfigInfo(
    uint32_t width, uint32_t height)
{
  return PipelineConfigInfo{};
}

std::vector<char> CoreVuPipeline::readFile(const std::string& filepath)
{
  std::ifstream file{
      filepath,
      std::ios::ate | std::ios::binary}; // go to the end|read as binary
  if (!file.is_open())
  {
    throw std::runtime_error("FAILURE::can't open file:" + filepath);
  }

  size_t file_size = static_cast<size_t>(file.tellg());
  std::vector<char> buffer(file_size);

  file.seekg(0); // go to start of file
  file.read(buffer.data(), file_size);

  file.close();

  return buffer;
}

void CoreVuPipeline::createGraphicsPipeline(
    const std::string& vert_filepath, const std::string& frag_filepath,
    const PipelineConfigInfo& config_info)
{
  auto vert_code = readFile(vert_filepath);
  auto frag_code = readFile(frag_filepath);

  std::cout << "file is read for " << vert_filepath << " "
            << (int)vert_code.size() << std::endl;
  std::cout << "file is read for " << frag_filepath << " "
            << (int)frag_code.size() << std::endl;
}

void CoreVuPipeline::createShaderModule(
    const std::vector<char>& code, VkShaderModule* shader_module)
{
  VkShaderModuleCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = code.size();
  create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

  if (vkCreateShaderModule(m_device.device(), &create_info, nullptr, shader_module) != VK_SUCCESS)
  {
    throw std::runtime_error("FAILURE::can't create shader module");
  }
}
