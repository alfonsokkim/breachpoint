#pragma once

#include <glm/mat4x4.hpp>

#include "core/Ray.h"

namespace craft::core {

// Encloses all eight corners after an affine model transform, including rotation.
[[nodiscard]] Aabb transformBounds(const Aabb& local, const glm::mat4& model);

}
