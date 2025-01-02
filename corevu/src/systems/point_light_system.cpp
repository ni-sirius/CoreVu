#include "systems/point_light_system.hpp"

// temp libs
#define GLM_FORCE_RADIANS           // to be sure that no change depending on system
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // instead of -1 to 1 ?
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
using namespace corevu;

PointLightSystem::PointLightSystem(
    CoreVuDevice& device, VkRenderPass render_pass,
    VkDescriptorSetLayout global_descriptor_set_layout)
  : m_corevu_device{device}
{
  createPipelineLayout(global_descriptor_set_layout);
  createPipeline(render_pass);
}

PointLightSystem::~PointLightSystem()
{
  vkDestroyPipelineLayout(m_corevu_device.device(), m_pipeline_layout, nullptr);
}

void PointLightSystem::createPipelineLayout(
    VkDescriptorSetLayout global_descriptor_set_layout)
{
  // TODO
  // VkPushConstantRange push_constant_range{};
  // push_constant_range.stageFlags =
  //     VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  // push_constant_range.offset =
  //     0; // if separate ranges for different stages are used.
  // push_constant_range.size = sizeof(SimplePushConstantData);

  std::vector<VkDescriptorSetLayout> descriptor_set_layouts = {
      global_descriptor_set_layout}; // temp in order to prepare for the
                                     // multiple layouts

  VkPipelineLayoutCreateInfo pipeline_layout_info{};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount =
      static_cast<uint32_t>(descriptor_set_layouts.size());
  pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
  pipeline_layout_info.pushConstantRangeCount =
      0; // 1 for one constant range for all stages
  pipeline_layout_info.pPushConstantRanges = 0;
      //&push_constant_range; // TODO what if 2 ranges? - do the same as for
                            // descriptor set layouts
  if (vkCreatePipelineLayout(
          m_corevu_device.device(), &pipeline_layout_info, nullptr,
          &m_pipeline_layout) != VK_SUCCESS)
  {
    throw std::runtime_error("FAILURE::can't create pipeline layout!");
  }
}

void PointLightSystem::createPipeline(VkRenderPass render_pass)
{
  assert(
      m_pipeline_layout != nullptr &&
      "Cannot create pipeline before pipeline layout");

  PipelineConfigInfo pipeline_config{};
  CoreVuPipeline::DefaultPipelineConfigInfo(pipeline_config);
  pipeline_config.renderPass = render_pass;
  pipeline_config.pipelineLayout = m_pipeline_layout;
  m_corevu_pipeline = std::make_unique<CoreVuPipeline>(
      m_corevu_device, pipeline_config,
      "C:/workspace/CoreVu/corevu/shaders/point_light.vert.spv",
      "C:/workspace/CoreVu/corevu/shaders/point_light.frag.spv");
}

void PointLightSystem::render(FrameInfo& frame_info)
{
  /* NOTE: for different shaders we would require to have different pipeleines,
   * WARN: not to rebind them often because it's expensive. */
  m_corevu_pipeline->Bind(frame_info.command_buffer);

  /* NOTE:
    Descriptor sets are bound in consequent order, which means that if we want
    to rebind set 0 we wouldn need to rebind all the sets after it. That's why
    it's important to have the most commoly used set at the beginning of the
    list. And others in the order of their usage.
   */
  vkCmdBindDescriptorSets(
      frame_info.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
      m_pipeline_layout, 0, 1, &frame_info.global_descriptor_set, 0,
      nullptr); // Bind the global descriptor set once to be used for all
                // objects.

  vkCmdDraw(frame_info.command_buffer, 6, 1, 0, 0);
}
