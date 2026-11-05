#include "core/MuzzleFlash.h"

#include <algorithm>
#include <cmath>

#include <glm/geometric.hpp>
#include <glm/vec4.hpp>

namespace craft::core {

glm::mat4 weaponFrame(const Ray& cameraRay) {
  const glm::vec3 forward = glm::normalize(cameraRay.direction);
  const glm::vec3 reference = std::abs(forward.x) + std::abs(forward.z) < 0.000001F ?
      glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);
  const glm::vec3 right = glm::normalize(glm::cross(forward, reference));
  const glm::vec3 up = glm::cross(right, forward);
  return {glm::vec4(right, 0), glm::vec4(up, 0),
           glm::vec4(-forward, 0), glm::vec4(cameraRay.origin, 1)};
}

glm::vec3 muzzlePosition(const Ray& cameraRay, const std::vector<Aabb>& solids) {
  const glm::vec3 mouth(weaponFrame(cameraRay) * glm::vec4(0.2F, -0.15F, -0.75F, 1));
  const glm::vec3 offset = mouth - cameraRay.origin;
  const auto wall = nearestBoxHit({cameraRay.origin, offset}, solids, 1.0F);
  const float fraction = wall ?
      std::max(0.0F, wall->t - 0.02F / glm::length(offset)) : 1.0F;
  return cameraRay.origin + offset * fraction;
}

void MuzzleFlash::update(float deltaSeconds) {
  if (std::isfinite(deltaSeconds) && deltaSeconds >= 0.0F) {
    remaining_ = std::max(0.0F, remaining_ - deltaSeconds);
  }
}

}
