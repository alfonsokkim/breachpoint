#pragma once

#include <vector>

#include "core/Ray.h"

namespace craft::core {

inline constexpr float kPlayerEyeHeight = 1.6F;
inline constexpr float kPlayerRadius = 0.25F;
inline constexpr float kPlayerHeight = 1.8F;

[[nodiscard]] Aabb playerBounds(glm::vec3 eye);

// Sweeps horizontal movement against solid boxes on a connected, flat floor.
[[nodiscard]] glm::vec3 movePlayer(glm::vec3 eye, glm::vec3 desired,
                                  const std::vector<Aabb>& solids);

}
