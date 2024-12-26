#pragma once

#include "corevu_model.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>

namespace corevu
{

struct TransformComponent
{
  glm::vec3
      translation{}; /* translation of positions shall not be affected w - 0. */
  glm::vec3 scale{1.f, 1.f, 1.f};
  glm::vec3 rotation{}; // all components in rad

  glm::mat4 ToMat4()
  {
    /* NOTE that's a dumb straighforward solution for rotation around arbitrary
    axis, below is more optimized solution with YXZ transform auto transform =
    glm::translate(glm::mat4x4{1.f}, translation);
    // For rotation is more common to use Tait-Btyan angles YXZ instad of proper
    Euler angles transform = glm::rotate(transform, rotation.y,
    glm::vec3{0.f, 1.f, 0.f}); transform = glm::rotate(transform, rotation.x,
    glm::vec3{1.f, 0.f, 0.f}); transform = glm::rotate(transform, rotation.z,
    glm::vec3{0.f, 0.f, 1.f}); transform = glm::scale(transform, scale);

    return transform; */

    // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
    // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
    // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
    /* NOTE: Extrinsic vs Intrinsic coordinate systems.
      to interprete rotation as Intrinsic read/apply transforms as YXZ and for
      Extrinsic ZXY(current impl)
     */
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    return glm::mat4{
        {
            scale.x * (c1 * c3 + s1 * s2 * s3),
            scale.x * (c2 * s3),
            scale.x * (c1 * s2 * s3 - c3 * s1),
            0.0f,
        },
        {
            scale.y * (c3 * s1 * s2 - c1 * s3),
            scale.y * (c2 * c3),
            scale.y * (c1 * c3 * s2 + s1 * s3),
            0.0f,
        },
        {
            scale.z * (c2 * s1),
            scale.z * (-s2),
            scale.z * (c1 * c2),
            0.0f,
        },
        {translation.x, translation.y, translation.z, 1.0f}};

    /* NOTE:
    glm::mat2 scale_mat = {{scale.x, .0f}, {.0f, scale.y}}; // glm take conlumns
    transform2d = rotation_mat *
           scale_mat; // scale transform is applied first then rotation then
                      // traslation. In glm::mat order of operation is reveresd.
                      */
  }

  glm::mat3
  GetNormalMatrix() // no optimal for large hierarchies with animated scale -
                    // use GPU evaluation instead. SEE shared code.
  {
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    const glm::vec3 inverse_scale = 1.f / scale;

    return glm::mat3{
        {
            inverse_scale.x * (c1 * c3 + s1 * s2 * s3),
            inverse_scale.x * (c2 * s3),
            inverse_scale.x * (c1 * s2 * s3 - c3 * s1),
        },
        {
            inverse_scale.y * (c3 * s1 * s2 - c1 * s3),
            inverse_scale.y * (c2 * c3),
            inverse_scale.y * (c1 * c3 * s2 + s1 * s3),
        },
        {
            inverse_scale.z * (c2 * s1),
            inverse_scale.z * (-s2),
            inverse_scale.z * (c1 * c2),
        }};

    // SAME AS return glm::mat3{glm::transpose(glm::inverse(ToMat4()))};
  }
};

// basic physics
struct RigidBody2dComponent
{
  glm::vec2 velocity{0.f};
  float mass{1.0f};
};

class CoreVuGameObject
{
public:
  using CoreVuUid = unsigned int;

  static CoreVuGameObject Create()
  {
    static CoreVuUid current_id = 0;
    return CoreVuGameObject{current_id++};
  }

  CoreVuGameObject(const CoreVuGameObject&) = delete;
  CoreVuGameObject& operator=(const CoreVuGameObject&) = delete;
  CoreVuGameObject(CoreVuGameObject&&) = default;
  CoreVuGameObject& operator=(CoreVuGameObject&&) = default;

  CoreVuUid GetUid() const
  {
    return m_uid;
  }

  // temporary open
  std::shared_ptr<CoreVuModel> model;
  glm::vec3 color;
  TransformComponent transform{};
  RigidBody2dComponent rigid_body;

private:
  CoreVuGameObject(const CoreVuUid& uid) : m_uid{uid}
  {
  }

  CoreVuUid m_uid;
};
} // namespace corevu