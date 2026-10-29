#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

#include "core/Ray.h"

namespace craft::core {

class Camera {
 public:
  explicit Camera(glm::vec3 position = {0.0F, 1.6F, 4.0F});

  [[nodiscard]] const glm::vec3& position() const noexcept;
  [[nodiscard]] float yawDegrees() const noexcept;
  [[nodiscard]] float pitchDegrees() const noexcept;
  [[nodiscard]] glm::vec3 forward() const;
  [[nodiscard]] Ray ray() const;
  [[nodiscard]] glm::mat4 viewProjection(float aspect) const;

  // Moves along the ground plane at four units per second, with unit input limits.
  void move(glm::vec2 input, float deltaSeconds);
  // Applies angle deltas in degrees and clamps pitch below vertical.
  void look(float yawDelta, float pitchDelta);
  // Lets collision logic supply a corrected position in world coordinates.
  void setPosition(glm::vec3 position);

 private:
  glm::vec3 position_{0.0F, 1.6F, 4.0F};
  float yaw_ = -90.0F;
  float pitch_ = -20.0F;
};

}
