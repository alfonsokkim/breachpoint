#include "core/ShotParticles.h"

#include <algorithm>
#include <cmath>

#include <glm/geometric.hpp>

namespace craft::core {

void ShotParticles::emit(glm::vec3 position, glm::vec3 direction, int count,
                         glm::vec4 colour, float size, float lifetime) {
  const float length = glm::length(direction);
  if (!std::isfinite(length) || length == 0.0F ||
      !std::isfinite(position.x) || !std::isfinite(position.y) ||
      !std::isfinite(position.z)) return;
  direction /= length;
  std::uniform_real_distribution<float> jitter(-0.5F, 0.5F);
  for (int i = 0; i < count && particles_.size() < kCapacity; ++i) {
    const glm::vec3 spread(jitter(random_), jitter(random_), jitter(random_));
    const float duration = lifetime * (0.8F + 0.4F * (jitter(random_) + 0.5F));
    particles_.push_back({position, direction * 1.5F + spread,
                          colour, size, duration, duration});
  }
}

void ShotParticles::emitMuzzle(glm::vec3 position, glm::vec3 forward) {
  emit(position, forward, 6, {1.0F, 0.7F, 0.25F, 0.9F}, 0.045F, 0.35F);
}

void ShotParticles::emitImpact(const ShotHit& hit) {
  if (hit.distance <= 0.0001F) return;
  emit(hit.position + hit.normal * 0.025F, hit.normal, 12,
       hit.kind == ShotHitKind::Solid ? glm::vec4(0.65F, 0.49F, 0.30F, 0.85F) :
                                       glm::vec4(0.72F, 0.74F, 0.77F, 0.85F),
       0.08F, 0.65F);
}

void ShotParticles::update(float deltaSeconds, const std::vector<Aabb>& solids) {
  if (!std::isfinite(deltaSeconds) || deltaSeconds <= 0.0F) return;
  const glm::vec3 gravity(0, -3, 0);
  for (auto& particle : particles_) {
    particle.remaining -= deltaSeconds;
    if (particle.remaining <= 0.0F) continue;
    const glm::vec3 motion = particle.velocity * deltaSeconds +
        gravity * (0.5F * deltaSeconds * deltaSeconds);
    if (nearestBoxHit({particle.position, motion}, solids, 1.0F)) {
      particle.remaining = 0.0F;
      continue;
    }
    particle.position += motion;
    particle.velocity += gravity * deltaSeconds;
  }
  particles_.erase(std::remove_if(particles_.begin(), particles_.end(),
      [](const ShotParticle& particle) { return particle.remaining <= 0.0F; }),
      particles_.end());
}

}
