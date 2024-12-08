#pragma once

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace corevu
{

/* GENERAL NOTE coord systems
  Summary of Vulkan Axes Orientation:
  Space              X-Axis	Y-Axis	Z-Axis	Notes
  Clip Space         Right	Up	Forward	Origin is center, range [-1, 1]
  NDC	Right	Up       Forward	Same as clip space after division
  Screen Space       Right	Down	N/A	Top-left is origin (Y-axis is flipped)
  World/Model Space  Right	Up	Forward/Backward	Depends on coordinate system

  Summary of OpenGL Axes Orientation:
  Space              X-Axis	Y-Axis	Z-Axis	Notes
  Clip Space         Right	Up	Backward	Origin is center, range [-1, 1]
  NDC	Right	Up       Backward	Same as clip space after division
  Window Space       Right	Up	N/A	Bottom-left is origin
  World/Model Space  Right	Up	Backward	Right-handed coordinate system
*/

class CoreVuCamera
{
public:
  void setOrthographicProjection(
      float left, float right, float top, float bottom, float near, float far);
  void setPerspectiveProjection(
      float fovy, float aspect, float near, float far);

  void setViewDirection(
      const glm::vec3& position, const glm::vec3& direction,
      const glm::vec3& up = glm::vec3{0.f, -1.f, 0.f} /* UP */);
  void setViewTarget(
      const glm::vec3& position, const glm::vec3& target,
      const glm::vec3& up = glm::vec3{0.f, -1.f, 0.f} /* UP */);
  // Use Euler/Tait-Bryan angles
  void setViewYXZ(const glm::vec3& position, const glm::vec3& rotation);

  const glm::mat4& getProjection() const
  {
    return m_projection_matrix;
  }
  const glm::mat4& getView() const
  {
    return m_view_matrix;
  }

private:
  glm::mat4 m_projection_matrix{1.f};
  glm::mat4 m_view_matrix{1.f};
};
} // namespace corevu