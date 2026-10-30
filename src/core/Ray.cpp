#include "core/Ray.h"

#include <algorithm>
#include <cmath>

namespace craft::core {

std::optional<float> intersectRayAabb(const Ray& ray, const Aabb& box,
                                      float maxT) {
  if (!std::isfinite(maxT) || maxT < 0.0F ||
      ray.direction == glm::vec3(0.0F)) {
    return std::nullopt;
  }

  double entry = 0.0;
  double exit = maxT;

  for (int axis = 0; axis < 3; ++axis) {
    const double origin = ray.origin[axis];
    const double direction = ray.direction[axis];
    const double minimum = box.minimum[axis];
    const double maximum = box.maximum[axis];

    if (!std::isfinite(origin) || !std::isfinite(direction) ||
        !std::isfinite(minimum) || !std::isfinite(maximum) || minimum > maximum) {
      return std::nullopt;
    }

    if (direction == 0.0) {
      if (origin < minimum || origin > maximum) {
        return std::nullopt;
      }
      continue;
    }

    double nearT = (minimum - origin) / direction;
    double farT = (maximum - origin) / direction;
    if (nearT > farT) {
      std::swap(nearT, farT);
    }

    entry = std::max(entry, nearT);
    exit = std::min(exit, farT);
    if (entry > exit) {
      return std::nullopt;
    }
  }

  return static_cast<float>(entry);
}

}
