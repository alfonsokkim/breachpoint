#pragma once

#include <optional>

#include "core/SpatialQueries.h"

namespace craft::core {

enum class ShotHitKind { Solid, Target };

struct ShotHit {
  ShotHitKind kind;
  std::size_t index;
  float distance;
  glm::vec3 position;
};

// Resolves a hitscan shot in world units; solids win equal-distance ties.
[[nodiscard]] std::optional<ShotHit> resolveShot(
    const Ray& ray, const std::vector<Aabb>& solids,
    const std::vector<Aabb>& targets, float range = 50.0F);

}
