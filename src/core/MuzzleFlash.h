#pragma once

#include <glm/mat4x4.hpp>

#include "core/SpatialQueries.h"

namespace craft::core {

// Camera-relative weapon transform; the barrel points along local -Z.
[[nodiscard]] glm::mat4 weaponFrame(const Ray& cameraRay);
[[nodiscard]] glm::vec3 muzzlePosition(const Ray& cameraRay,
                                      const std::vector<Aabb>& solids);

class MuzzleFlash {
 public:
  static constexpr float kDuration = 0.12F;
  void trigger() noexcept { remaining_ = kDuration; }
  void update(float deltaSeconds);
  void reset() noexcept { remaining_ = 0.0F; }
  [[nodiscard]] float intensity() const noexcept { return remaining_ / kDuration; }

 private:
  float remaining_ = 0.0F;
};

}
