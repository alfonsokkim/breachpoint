#include "core/Shooting.h"

#include <cmath>
#include <limits>

#include <glm/geometric.hpp>

namespace craft::core {

namespace {
glm::vec3 entryNormal(const Ray& ray, const Aabb& box) {
  double entry = -std::numeric_limits<double>::infinity();
  glm::vec3 normal(0);
  for (int axis = 0; axis < 3; ++axis) {
    if (ray.direction[axis] == 0.0F) continue;
    const bool positive = ray.direction[axis] > 0.0F;
    const double face = positive ? box.minimum[axis] : box.maximum[axis];
    const double nearT = (face - ray.origin[axis]) / ray.direction[axis];
    if (nearT > entry) {
      entry = nearT;
      normal = glm::vec3(0);
      normal[axis] = positive ? -1.0F : 1.0F;
    }
  }
  return normal;
}
}

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
                   shot.origin + shot.direction * target->t,
                   entryNormal(shot, targets[target->index])};
  }
  if (solid) {
    return ShotHit{ShotHitKind::Solid, solid->index, solid->t,
                   shot.origin + shot.direction * solid->t,
                   entryNormal(shot, solids[solid->index])};
  }
  return std::nullopt;
}

}
