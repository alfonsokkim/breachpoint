#pragma once

#include <random>
#include <vector>

#include <glm/vec4.hpp>

#include "core/Shooting.h"

namespace craft::core {

struct ShotParticle {
  glm::vec3 position;
  glm::vec3 velocity;
  glm::vec4 colour;
  float size;
  float remaining;
  float lifetime;
};

// Short-lived muzzle sparks and impact dust, bounded to 128 live particles.
class ShotParticles {
 public:
  static constexpr std::size_t kCapacity = 128;
  void emitMuzzle(glm::vec3 position, glm::vec3 forward);
  void emitImpact(const ShotHit& hit);
  void update(float deltaSeconds, const std::vector<Aabb>& solids);
  void clear() noexcept { particles_.clear(); }
  [[nodiscard]] const std::vector<ShotParticle>& live() const noexcept { return particles_; }

 private:
  void emit(glm::vec3 position, glm::vec3 direction, int count,
             glm::vec4 colour, float size, float lifetime);
  std::mt19937 random_{3421};
  std::vector<ShotParticle> particles_;
};

}
