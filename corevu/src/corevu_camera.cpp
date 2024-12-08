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
  /** NOTE From CoreOne
  * _projectionMat = glm::perspective(
     glm::radians(_fov),
     static_cast<float>(framebufferWidth) / frameBufferHeight,
     _nearPlane,
     _farPlane);
  */
  assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
  const float tanHalfFovy = tan(fovy / 2.f);
  m_projection_matrix = glm::mat4{0.0f};
  m_projection_matrix[0][0] = 1.f / (aspect * tanHalfFovy);
  m_projection_matrix[1][1] = 1.f / (tanHalfFovy);
  m_projection_matrix[2][2] = far / (far - near);
  m_projection_matrix[2][3] = 1.f;
  m_projection_matrix[3][2] = -(far * near) / (far - near);
}
void CoreVuCamera::setViewDirection(
    const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up)
{
  // construction unit vectors lengths 1, 90deg angle to each other
  const glm::vec3 w{glm::normalize(direction)};
  const glm::vec3 u{glm::normalize(glm::cross(w, up))};
  const glm::vec3 v{glm::cross(w, u)};

  m_view_matrix = glm::mat4{1.f};
  m_view_matrix[0][0] = u.x;
  m_view_matrix[1][0] = u.y;
  m_view_matrix[2][0] = u.z;
  m_view_matrix[0][1] = v.x;
  m_view_matrix[1][1] = v.y;
  m_view_matrix[2][1] = v.z;
  m_view_matrix[0][2] = w.x;
  m_view_matrix[1][2] = w.y;
  m_view_matrix[2][2] = w.z;
  m_view_matrix[3][0] = -glm::dot(u, position);
  m_view_matrix[3][1] = -glm::dot(v, position);
  m_view_matrix[3][2] = -glm::dot(w, position);
}
void CoreVuCamera::setViewTarget(
    const glm::vec3& position, const glm::vec3& target, const glm::vec3& up)
{
  /** NOTE
   *  glm::lookAt() from CoreOne */
  setViewDirection(position, target - position, up);
}
void CoreVuCamera::setViewYXZ(
    const glm::vec3& position, const glm::vec3& rotation)
{
  /* use inverse rotation matrix to transform from camera direction to canonical
   * rotation direction. inverse ehre is the same as transpose(change columns
   * and rows) */
  const float c3 = glm::cos(rotation.z);
  const float s3 = glm::sin(rotation.z);
  const float c2 = glm::cos(rotation.x);
  const float s2 = glm::sin(rotation.x);
  const float c1 = glm::cos(rotation.y);
  const float s1 = glm::sin(rotation.y);
  const glm::vec3 u{
      (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
  const glm::vec3 v{
      (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
  const glm::vec3 w{(c2 * s1), (-s2), (c1 * c2)};
  m_view_matrix = glm::mat4{1.f};
  m_view_matrix[0][0] = u.x;
  m_view_matrix[1][0] = u.y;
  m_view_matrix[2][0] = u.z;
  m_view_matrix[0][1] = v.x;
  m_view_matrix[1][1] = v.y;
  m_view_matrix[2][1] = v.z;
  m_view_matrix[0][2] = w.x;
  m_view_matrix[1][2] = w.y;
  m_view_matrix[2][2] = w.z;
  m_view_matrix[3][0] = -glm::dot(u, position);
  m_view_matrix[3][1] = -glm::dot(v, position);
  m_view_matrix[3][2] = -glm::dot(w, position);
}
} // namespace corevu