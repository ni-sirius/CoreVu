#include <corevu_pipeline.hpp>

#include <fstream>
#include <stdexcept>

#include <iostream>

corevu::CoreVuPipeline::CoreVuPipeline(
    const std::string& vert_filepath, const std::string& frag_filepath)
{
  createGraphicsPipeline(vert_filepath, frag_filepath);
}

std::vector<char> corevu::CoreVuPipeline::readFile(const std::string& filepath)
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

void corevu::CoreVuPipeline::createGraphicsPipeline(
    const std::string& vert_filepath, const std::string& frag_filepath)
{
  auto vert_code = readFile(vert_filepath);
  auto frag_code = readFile(frag_filepath);

  std::cout << "file is read for " << vert_filepath << " "
            << (int)vert_code.size() << std::endl;
  std::cout << "file is read for " << frag_filepath << " "
            << (int)frag_code.size() << std::endl;
}
