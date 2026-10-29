#pragma once

#include <cstddef>
#include <limits>
#include <optional>
#include <vector>

#include "core/Ray.h"

namespace craft::core {

struct BoxHit {
  std::size_t index;
  float t;
};

// Finds the nearest world-space box in [0, maxT], keeping the first on ties.
[[nodiscard]] std::optional<BoxHit> nearestBoxHit(
    const Ray& ray, const std::vector<Aabb>& boxes,
    float maxT = std::numeric_limits<float>::max());

// Returns false if a solid box touches the segment, including either endpoint.
[[nodiscard]] bool hasLineOfSight(glm::vec3 from, glm::vec3 to,
                                   const std::vector<Aabb>& occluders);

}
