#include "systems/render_system.hpp"

// temp libs
#define GLM_FORCE_RADIANS           // to be sure that no change depending on system
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // instead of -1 to 1 ?
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
using namespace corevu;

struct SimplePushConstantData // NOTE : ALL push data constants together are
                              // limited to 128 bytes space! But it's quite
                              // handy for storing transformation matrices.
{
  glm::mat4 model_matrix{1.f};
  glm::mat4 normal_matrix{1.f};

  /* NOTE an 2d implementation with important note
  glm::mat2 transform{1.f};
  glm::vec2 offset; // 4 bytes * 2 = 8 bytes
  alignas(16) glm::vec3
      color; // w/o alignas will start with 9th byte // vulkan require
             // alignment of 4*size(val) for 3&4 component vectors. -> need to
             // add offset 16 bytes instead of (4bytes*2)of vec2 member offset.
             */
};

RenderSystem::RenderSystem(
    CoreVuDevice& device, VkRenderPass render_pass,
    VkDescriptorSetLayout global_descriptor_set_layout)
  : m_corevu_device{device}
{
  createPipelineLayout(global_descriptor_set_layout);
  createPipeline(render_pass);
}

RenderSystem::~RenderSystem()
{
  vkDestroyPipelineLayout(m_corevu_device.device(), m_pipeline_layout, nullptr);
}

void RenderSystem::createPipelineLayout(
    VkDescriptorSetLayout global_descriptor_set_layout)
{
  VkPushConstantRange push_constant_range{};
  push_constant_range.stageFlags =
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  push_constant_range.offset =
      0; // if separate ranges for different stages are used.
  push_constant_range.size = sizeof(SimplePushConstantData);

  std::vector<VkDescriptorSetLayout> descriptor_set_layouts = {
      global_descriptor_set_layout}; // temp in order to prepare for the
                                     // multiple layouts

  VkPipelineLayoutCreateInfo pipeline_layout_info{};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount =
      static_cast<uint32_t>(descriptor_set_layouts.size());
  pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
  pipeline_layout_info.pushConstantRangeCount =
      1; // 1 for one constant range for all stages
  pipeline_layout_info.pPushConstantRanges =
      &push_constant_range; // TODO what if 2 ranges? - do the same as for
                            // descriptor set layouts
  if (vkCreatePipelineLayout(
          m_corevu_device.device(), &pipeline_layout_info, nullptr,
          &m_pipeline_layout) != VK_SUCCESS)
  {
    throw std::runtime_error("FAILURE::can't create pipeline layout!");
  }
}

void RenderSystem::createPipeline(VkRenderPass render_pass)
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
      "C:/workspace/CoreVu/corevu/shaders/simple_shader.vert.spv",
      "C:/workspace/CoreVu/corevu/shaders/simple_shader.frag.spv");
}

void RenderSystem::renderGameObjects(
    FrameInfo& frame_info, std::vector<CoreVuGameObject>& game_objects)
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

  for (auto& obj : game_objects)
  {
    // TEST ROTATION FOR ALL GAME OBJECTS(TODO remove)
    // obj.transform.rotation.y =
    //     glm::mod(obj.transform.rotation.y + 0.01f, glm::two_pi<float>());
    // obj.transform.rotation.x =
    //     glm::mod(obj.transform.rotation.x + 0.005f, glm::two_pi<float>());
    // obj.transform.rotation.z =
    //     glm::mod(obj.transform.rotation.z + 0.0001f, glm::two_pi<float>());

    SimplePushConstantData push{};
    push.normal_matrix = obj.transform.GetNormalMatrix();
    push.model_matrix = obj.transform.ToMat4();

    vkCmdPushConstants(
        frame_info.command_buffer, m_pipeline_layout,
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0,
        sizeof(SimplePushConstantData), &push);

    obj.model->Bind(frame_info.command_buffer);
    obj.model->Draw(frame_info.command_buffer);
  }
}
