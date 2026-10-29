#include "core/Camera.h"

#include <algorithm>
#include <cmath>

#include <glm/common.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

namespace craft::core {

Camera::Camera(glm::vec3 position) {
  setPosition(position);
}

const glm::vec3& Camera::position() const noexcept { return position_; }
float Camera::yawDegrees() const noexcept { return yaw_; }
float Camera::pitchDegrees() const noexcept { return pitch_; }

glm::vec3 Camera::forward() const {
  const float yaw = glm::radians(yaw_);
  const float pitch = glm::radians(pitch_);
  return glm::normalize(glm::vec3(std::cos(yaw) * std::cos(pitch),
                                   std::sin(pitch),
                                   std::sin(yaw) * std::cos(pitch)));
}

Ray Camera::ray() const { return {position_, forward()}; }

glm::mat4 Camera::viewProjection(float aspect) const {
  if (!std::isfinite(aspect) || aspect <= 0.0F) {
    aspect = 1.0F;
  }
  return glm::perspective(glm::radians(55.0F), aspect, 0.1F, 100.0F) *
         glm::lookAt(position_, position_ + forward(), glm::vec3(0, 1, 0));
}

void Camera::move(glm::vec2 input, float deltaSeconds) {
  if (!std::isfinite(deltaSeconds) || deltaSeconds <= 0.0F ||
      !std::isfinite(input.x) || !std::isfinite(input.y)) {
    return;
  }
  input = glm::clamp(input, glm::vec2(-1.0F), glm::vec2(1.0F));
  if (glm::dot(input, input) > 1.0F) {
    input = glm::normalize(input);
  }
  const float yaw = glm::radians(yaw_);
  const glm::vec3 ahead(std::cos(yaw), 0.0F, std::sin(yaw));
  const glm::vec3 right(-std::sin(yaw), 0.0F, std::cos(yaw));
  setPosition(position_ + (right * input.x + ahead * input.y) *
                              (4.0F * deltaSeconds));
}

void Camera::look(float yawDelta, float pitchDelta) {
  if (!std::isfinite(yawDelta) || !std::isfinite(pitchDelta)) {
    return;
  }
  yaw_ = static_cast<float>(std::remainder(static_cast<double>(yaw_) + yawDelta,
                                          360.0));
  pitch_ = static_cast<float>(std::clamp(static_cast<double>(pitch_) + pitchDelta,
                                        -89.0, 89.0));
}

void Camera::setPosition(glm::vec3 position) {
  if (std::isfinite(position.x) && std::isfinite(position.y) &&
      std::isfinite(position.z)) {
    position_ = position;
  }
}

}
