#include "core/SpatialQueries.h"

#include <cmath>

namespace craft::core {

namespace {
bool finite(glm::vec3 point) {
  return std::isfinite(point.x) && std::isfinite(point.y) &&
         std::isfinite(point.z);
}
}

std::optional<BoxHit> nearestBoxHit(const Ray& ray,
                                    const std::vector<Aabb>& boxes, float maxT) {
  std::optional<BoxHit> nearest;
  for (std::size_t index = 0; index < boxes.size(); ++index) {
    const auto t = intersectRayAabb(ray, boxes[index], maxT);
    if (t && (!nearest || *t < nearest->t)) {
      nearest = BoxHit{index, *t};
    }
  }
  return nearest;
}

bool hasLineOfSight(glm::vec3 from, glm::vec3 to,
                      const std::vector<Aabb>& occluders) {
  const glm::vec3 delta = to - from;
  if (!finite(from) || !finite(to) || !finite(delta)) {
    return false;
  }
  const bool samePoint = from == to;
  const Ray segment{from, samePoint ? glm::vec3(1, 0, 0) : delta};
  return !nearestBoxHit(segment, occluders, samePoint ? 0.0F : 1.0F);
}

}
