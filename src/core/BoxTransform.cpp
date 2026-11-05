#include "core/BoxTransform.h"

#include <limits>

#include <glm/common.hpp>
#include <glm/vec4.hpp>

namespace craft::core {

Aabb transformBounds(const Aabb& local, const glm::mat4& model) {
  const float infinity = std::numeric_limits<float>::infinity();
  Aabb world{glm::vec3(infinity), glm::vec3(-infinity)};
  for (int corner = 0; corner < 8; ++corner) {
    const glm::vec3 point{
        corner & 1 ? local.maximum.x : local.minimum.x,
        corner & 2 ? local.maximum.y : local.minimum.y,
        corner & 4 ? local.maximum.z : local.minimum.z};
    const glm::vec3 transformed(model * glm::vec4(point, 1.0F));
    world.minimum = glm::min(world.minimum, transformed);
    world.maximum = glm::max(world.maximum, transformed);
  }
  return world;
}

}
