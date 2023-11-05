#pragma once

#include <string>
#include <vector>

namespace corevu
{
class CoreVuPipeline
{
public:
  CoreVuPipeline(
      const std::string& vert_filepath, const std::string& frag_filepath);

private:
  static std::vector<char> readFile(const std::string& filepath);
  void createGraphicsPipeline(
      const std::string& vert_filepath, const std::string& frag_filepath);
};
} // namespace corevu