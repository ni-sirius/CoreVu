#include <corevu_camera.hpp>

// std
#include <cassert>
#include <limits>

// glm test
// #include <glm/ext/matrix_transform.inl> adds  glm::lookAt()

namespace corevu
{

void CoreVuCamera::setOrthographicProjection(
    float left, float right, float top, float bottom, float near, float far)
{
  m_projection_matrix = glm::mat4{1.0f};
  m_projection_matrix[0][0] = 2.f / (right - left);
  m_projection_matrix[1][1] = 2.f / (bottom - top);
  m_projection_matrix[2][2] = 1.f / (far - near);
  m_projection_matrix[3][0] = -(right + left) / (right - left);
  m_projection_matrix[3][1] = -(bottom + top) / (bottom - top);
  m_projection_matrix[3][2] = -near / (far - near);
}

void CoreVuCamera::setPerspectiveProjection(
    float fovy, float aspect, float near, float far)
{
  assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
  const float tanHalfFovy = tan(fovy / 2.f);
  m_projection_matrix = glm::mat4{0.0f};
  m_projection_matrix[0][0] = 1.f / (aspect * tanHalfFovy);
  m_projection_matrix[1][1] = 1.f / (tanHalfFovy);
  m_projection_matrix[2][2] = far / (far - near);
  m_projection_matrix[2][3] = 1.f;
  m_projection_matrix[3][2] = -(far * near) / (far - near);
}
} // namespace corevu