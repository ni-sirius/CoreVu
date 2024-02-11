#pragma once
#include <corevu_gameobject.hpp>

// std
#include <vector>

namespace corevu
{
class GravityPhysicsSystem
{
public:
  GravityPhysicsSystem(float strength) : strengthGravity{strength}
  {
  }

  const float strengthGravity;

  // dt stands for delta time, and specifies the amount of time to advance the
  // simulation substeps is how many intervals to divide the forward time step
  // in. More substeps result in a more stable simulation, but takes longer to
  // compute
  void update(
      std::vector<CoreVuGameObject>& objs, float dt, unsigned int substeps = 1)
  {
    const float stepDelta = dt / substeps;
    for (int i = 0; i < substeps; i++)
    {
      stepSimulation(objs, stepDelta);
    }
  }

  glm::vec2 computeForce(
      CoreVuGameObject& fromObj, CoreVuGameObject& toObj) const
  {
    auto offset = fromObj.transform.translation - toObj.transform.translation;
    float distanceSquared = glm::dot(offset, offset);

    // clown town - just going to return 0 if objects are too close together...
    if (glm::abs(distanceSquared) < 1e-10f) { return {.0f, .0f}; }

    float force = strengthGravity * toObj.rigid_body.mass *
                  fromObj.rigid_body.mass / distanceSquared;
    return force * offset / glm::sqrt(distanceSquared);
  }

private:
  void stepSimulation(std::vector<CoreVuGameObject>& physicsObjs, float dt)
  {
    // Loops through all pairs of objects and applies attractive force between
    // them
    for (auto iterA = physicsObjs.begin(); iterA != physicsObjs.end(); ++iterA)
    {
      auto& objA = *iterA;
      for (auto iterB = iterA; iterB != physicsObjs.end(); ++iterB)
      {
        if (iterA == iterB) continue;
        auto& objB = *iterB;

        auto force = computeForce(objA, objB);
        objA.rigid_body.velocity += dt * -force / objA.rigid_body.mass;
        objB.rigid_body.velocity += dt * force / objB.rigid_body.mass;
      }
    }

    // update each objects position based on its final velocity
    for (auto& obj : physicsObjs)
    {
      obj.transform.translation += dt * obj.rigid_body.velocity;
    }
  }
};
} // namespace corevu