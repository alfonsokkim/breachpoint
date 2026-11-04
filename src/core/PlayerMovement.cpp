#include "core/PlayerMovement.h"

#include <algorithm>
#include <cmath>

namespace craft::core {
namespace {
constexpr float kTolerance = 0.00001F;

bool finite(glm::vec3 point) {
  return std::isfinite(point.x) && std::isfinite(point.y) &&
         std::isfinite(point.z);
}

bool overlap(const Aabb& a, const Aabb& b, int axis) {
  return a.minimum[axis] < b.maximum[axis] - kTolerance &&
         a.maximum[axis] > b.minimum[axis] + kTolerance;
}

float groundLimit(glm::vec3 eye, int axis, float desired,
                  const std::vector<Aabb>& solids) {
  const bool positive = desired > eye[axis];
  const int across = axis == 0 ? 2 : 0;
  const float floor = eye.y - kPlayerEyeHeight;
  for (float x : {-kPlayerRadius, kPlayerRadius}) {
    for (float z : {-kPlayerRadius, kPlayerRadius}) {
      const glm::vec3 offset(x, 0.0F, z);
      const glm::vec3 corner = eye + offset;
      float edge = corner[axis];
      bool supported = false;
      bool extended;
      do {
        extended = false;
        for (const auto& box : solids) {
          if (std::abs(box.maximum.y - floor) > kTolerance ||
              corner[across] < box.minimum[across] ||
              corner[across] > box.maximum[across] ||
              edge < box.minimum[axis] - kTolerance ||
              edge > box.maximum[axis] + kTolerance) {
            continue;
          }
          supported = true;
          const float next = positive ? std::max(edge, box.maximum[axis])
                                      : std::min(edge, box.minimum[axis]);
          if (next != edge) {
            edge = next;
            extended = true;
          }
        }
      } while (extended);
      if (!supported) {
        return eye[axis];
      }
      desired = positive ? std::min(desired, edge - offset[axis])
                         : std::max(desired, edge - offset[axis]);
    }
  }
  return desired;
}
}

Aabb playerBounds(glm::vec3 eye) {
  const glm::vec3 feet = eye - glm::vec3(0.0F, kPlayerEyeHeight, 0.0F);
  return {feet - glm::vec3(kPlayerRadius, 0.0F, kPlayerRadius),
          feet + glm::vec3(kPlayerRadius, kPlayerHeight, kPlayerRadius)};
}

glm::vec3 movePlayer(glm::vec3 eye, glm::vec3 desired,
                     const std::vector<Aabb>& solids) {
  if (!finite(eye) || !finite(desired)) {
    return eye;
  }
  for (const auto& box : solids) {
    const auto body = playerBounds(eye);
    if (overlap(body, box, 0) && overlap(body, box, 1) && overlap(body, box, 2)) {
      return eye;
    }
  }
  for (int axis : {0, 2}) {
    const auto body = playerBounds(eye);
    const bool positive = desired[axis] > eye[axis];
    float destination = desired[axis];
    for (const auto& box : solids) {
      if (!overlap(body, box, 1) || !overlap(body, box, axis == 0 ? 2 : 0)) {
        continue;
      }
      if (positive && body.maximum[axis] <= box.minimum[axis] + kTolerance) {
        destination = std::min(destination, box.minimum[axis] - kPlayerRadius);
      } else if (!positive && body.minimum[axis] >= box.maximum[axis] - kTolerance) {
        destination = std::max(destination, box.maximum[axis] + kPlayerRadius);
      }
    }
    if (destination != eye[axis]) {
      eye[axis] = groundLimit(eye, axis, destination, solids);
    }
  }
  return eye;
}

}
