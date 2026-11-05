#pragma once

#include <vector>

#include <glm/vec3.hpp>

#include "core/Ray.h"

namespace craft::core {

enum class GroundMaterial { Concrete, Wall, Cover, Yellow, Red, Blue };
struct GroundBlock { glm::ivec3 cell; GroundMaterial material; };
struct TargetSpawn { glm::vec3 feet; bool civilian; };
struct GroundSpot { glm::vec3 position; glm::vec3 aim; };
struct TrainingGround {
  const char* name;
  glm::vec3 spawn;
  float yaw;
  std::vector<GroundBlock> blocks;
  std::vector<TargetSpawn> targets;
  std::vector<GroundSpot> lights;
};

// A single-storey concrete training courtyard with cover and floodlights.
[[nodiscard]] TrainingGround makeTrainingGround(int level);

}
