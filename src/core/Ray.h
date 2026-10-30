#pragma once

#include <limits>
#include <optional>

#include <glm/vec3.hpp>

namespace craft::core {

struct Ray {
  glm::vec3 origin;
  glm::vec3 direction;
};

struct Aabb {
  glm::vec3 minimum;
  glm::vec3 maximum;
};

// Returns the first t in [0, maxT] for origin + t * direction, or no hit.
[[nodiscard]] std::optional<float> intersectRayAabb(
    const Ray& ray, const Aabb& box,
    float maxT = std::numeric_limits<float>::max());

}
