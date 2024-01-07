#pragma once

#include "corevu_model.hpp"

#include <memory>

namespace corevu
{

struct Transform2dComponent
{
  glm::vec2 translation{0.f, 0.f}; // pos offset
  glm::vec2 scale{1.f, 1.f};
  float rotation = 0.f; // rad

  glm::mat2 ToMat2()
  {
    const float c = glm::cos(rotation);
    const float s = glm::sin(rotation);
    glm::mat2 rotation_mat = {{c, s}, {-s, c}};

    glm::mat2 scale_mat = {{scale.x, .0f}, {.0f, scale.y}}; // glm take conlumns
    return rotation_mat *
           scale_mat; // scale transform is applied first then rotation then
                      // traslation. In glm order of operation is reveresd.
  }
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
  Transform2dComponent transform;

private:
  CoreVuGameObject(const CoreVuUid& uid) : m_uid{uid}
  {
  }

  CoreVuUid m_uid;
};
} // namespace corevu