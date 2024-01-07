#pragma once

#include "corevu_model.hpp"

#include <memory>

namespace corevu
{

struct Transform2dComponent
{
  glm::vec2 translation{}; // pos offset

  glm::mat2 ToMat2() {return glm::mat2{1.f};}
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

  CoreVuUid GetUid() const { return m_uid;}

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