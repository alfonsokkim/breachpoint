#include "core/Shooting.h"

#include <cmath>

#include <glm/geometric.hpp>

namespace craft::core {

std::optional<ShotHit> resolveShot(const Ray& ray,
                                   const std::vector<Aabb>& solids,
                                   const std::vector<Aabb>& targets,
                                   float range) {
  const double length = glm::length(glm::dvec3(ray.direction));
  if (!std::isfinite(range) || range < 0.0F ||
      !std::isfinite(length) || length == 0.0) {
    return std::nullopt;
  }
  const Ray shot{ray.origin, glm::vec3(glm::dvec3(ray.direction) / length)};
  const auto solid = nearestBoxHit(shot, solids, range);
  const auto target = nearestBoxHit(shot, targets, range);
  if (target && (!solid || target->t < solid->t)) {
    return ShotHit{ShotHitKind::Target, target->index, target->t,
                   shot.origin + shot.direction * target->t};
  }
  if (solid) {
    return ShotHit{ShotHitKind::Solid, solid->index, solid->t,
                   shot.origin + shot.direction * solid->t};
  }
  return std::nullopt;
}

}
