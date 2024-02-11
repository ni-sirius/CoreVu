#pragma once
#include <corevu_gameobject.hpp>
#include "gravity_system.hpp"

// std
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace corevu
{
class Vec2FieldSystem
{
public:
  void update(
      const GravityPhysicsSystem& physicsSystem,
      std::vector<CoreVuGameObject>& physicsObjs,
      std::vector<CoreVuGameObject>& vectorField)
  {
    // For each field line we caluclate the net graviation force for that point
    // in space
    for (auto& vf : vectorField)
    {
      glm::vec2 direction{};
      for (auto& obj : physicsObjs)
      {
        direction += physicsSystem.computeForce(obj, vf);
      }

      // This scales the length of the field line based on the log of the length
      // values were chosen just through trial and error based on what i liked
      // the look of and then the field line is rotated to point in the
      // direction of the field
      vf.transform.scale.x =
          0.005f +
          0.045f *
              glm::clamp(glm::log(glm::length(direction) + 1) / 3.f, 0.f, 1.f);
      vf.transform.rotation = atan2(direction.y, direction.x);
    }
  }
};

std::unique_ptr<CoreVuModel> createSquareModel(
    CoreVuDevice& device, glm::vec2 offset)
{
  std::vector<CoreVuModel::Vertex> vertices = {
      {{-0.5f, -0.5f}}, {{0.5f, 0.5f}},  {{-0.5f, 0.5f}},
      {{-0.5f, -0.5f}}, {{0.5f, -0.5f}}, {{0.5f, 0.5f}}, //
  };
  for (auto& v : vertices)
  {
    v.position += offset;
  }
  return std::make_unique<CoreVuModel>(device, vertices);
}

std::unique_ptr<CoreVuModel> createCircleModel(
    CoreVuDevice& device, unsigned int numSides)
{
  std::vector<CoreVuModel::Vertex> uniqueVertices{};
  for (int i = 0; i < numSides; i++)
  {
    float angle = i * glm::two_pi<float>() / numSides;
    uniqueVertices.push_back({.position = {glm::cos(angle), glm::sin(angle)}});
  }
  uniqueVertices.push_back({}); // adds center vertex at 0, 0

  std::vector<CoreVuModel::Vertex> vertices{};
  for (int i = 0; i < numSides; i++)
  {
    vertices.push_back(uniqueVertices[i]);
    vertices.push_back(uniqueVertices[(i + 1) % numSides]);
    vertices.push_back(uniqueVertices[numSides]);
  }
  return std::make_unique<CoreVuModel>(device, vertices);
}
} // namespace corevu