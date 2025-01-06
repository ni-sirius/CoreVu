#include <systems/texture_render_system.hpp>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace corevu
{

struct TexturePushConstantData
{
  glm::mat4 modelMatrix{1.f};
  glm::mat4 normalMatrix{1.f};
};

TextureRenderSystem::TextureRenderSystem(
    CoreVuDevice& device, VkRenderPass renderPass,
    VkDescriptorSetLayout globalSetLayout)
  : m_device{device}
{
  createPipelineLayout(globalSetLayout);
  createPipeline(renderPass);
}

TextureRenderSystem::~TextureRenderSystem()
{
  vkDestroyPipelineLayout(m_device.device(), m_pipeline_layout, nullptr);
}

void TextureRenderSystem::createPipelineLayout(
    VkDescriptorSetLayout globalSetLayout)
{
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags =
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(TexturePushConstantData);

  m_render_system_layout = CoreVuDescriptorSetLayout::Builder(m_device)
                               .addBinding(
                                   0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                   VK_SHADER_STAGE_FRAGMENT_BIT)
                               .build();

  std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
      globalSetLayout, m_render_system_layout->getDescriptorSetLayout()};

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount =
      static_cast<uint32_t>(descriptorSetLayouts.size());
  pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
  if (vkCreatePipelineLayout(
          m_device.device(), &pipelineLayoutInfo, nullptr,
          &m_pipeline_layout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create pipeline layout!");
  }
}

void TextureRenderSystem::createPipeline(VkRenderPass renderPass)
{
  assert(
      m_pipeline_layout != nullptr &&
      "Cannot create pipeline before pipeline layout");

  PipelineConfigInfo pipelineConfig{};
  CoreVuPipeline::DefaultPipelineConfigInfo(pipelineConfig);
  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = m_pipeline_layout;
  m_pipeline = std::make_unique<CoreVuPipeline>(
      m_device, pipelineConfig,
      "C:/workspace/CoreVu/corevu/shaders/texture_shader.vert.spv",
      "C:/workspace/CoreVu/corevu/shaders/texture_shader.frag.spv");
}

void TextureRenderSystem::renderGameObjects(FrameInfo& frameInfo)
{
  m_pipeline->Bind(frameInfo.command_buffer);

  vkCmdBindDescriptorSets(
      frameInfo.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
      m_pipeline_layout, 0, 1, &frameInfo.global_descriptor_set, 0, nullptr);

  for (auto& kv : frameInfo.game_objects)
  {
    auto& obj = kv.second;

    // skip objects that don't have both a model and texture
    if (obj.model == nullptr || obj.diffuse_map == nullptr)
      continue;

    // writing descriptor set each frame can slow performance
    // would be more efficient to implement some sort of caching
    auto imageInfo = obj.diffuse_map->getImageInfo();
    VkDescriptorSet descriptorSet1;
    CoreVuDescriptorWriter(
        *m_render_system_layout, frameInfo.frame_descriptor_pool)
        .writeImage(0, &imageInfo)
        .build(descriptorSet1);

    vkCmdBindDescriptorSets(
        frameInfo.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_pipeline_layout,
        1, // first set
        1, // set count
        &descriptorSet1, 0, nullptr);

    TexturePushConstantData push{};
    push.modelMatrix = obj.transform.ToMat4();
    push.normalMatrix = obj.transform.GetNormalMatrix();

    vkCmdPushConstants(
        frameInfo.command_buffer, m_pipeline_layout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
        sizeof(TexturePushConstantData), &push);

    obj.model->Bind(frameInfo.command_buffer);
    obj.model->Draw(frameInfo.command_buffer);
  }
}

} // namespace corevu
