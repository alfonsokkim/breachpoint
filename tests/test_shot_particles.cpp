#include <limits>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "core/ShotParticles.h"

using namespace craft::core;
using Catch::Approx;

TEST_CASE("Particle motion and lifetime use elapsed time") {
  ShotParticles particles;
  particles.emitMuzzle({0, 2, 0}, {0, 0, -1});
  REQUIRE(particles.live().size() == 6);
  const auto initial = particles.live().front();
  particles.update(0.1F, {});
  const auto& moved = particles.live().front();
  REQUIRE(moved.position.x == Approx(initial.position.x + initial.velocity.x * 0.1F));
  REQUIRE(moved.position.y == Approx(initial.position.y + initial.velocity.y * 0.1F - 0.015F));
  REQUIRE(moved.remaining == Approx(initial.remaining - 0.1F));
  particles.update(1, {});
  REQUIRE(particles.live().empty());
}

TEST_CASE("Particles cannot cross a nearer solid even in one large step") {
  ShotParticles particles;
  particles.emitMuzzle({0, 2, 0}, {0, 0, -1});
  particles.update(0.1F, {{{-5, 0, -0.06F}, {5, 4, -0.05F}}});
  REQUIRE(particles.live().empty());
}

TEST_CASE("Particle bursts are bounded and can be cleared for a reset") {
  ShotParticles particles;
  for (int i = 0; i < 100; ++i) particles.emitMuzzle({0, 2, 0}, {0, 0, -1});
  REQUIRE(particles.live().size() == ShotParticles::kCapacity);
  particles.clear();
  REQUIRE(particles.live().empty());
  particles.emitMuzzle({0, 2, 0}, {0, 0, -1});
  REQUIRE(particles.live().size() == 6);
}

TEST_CASE("Impact dust starts outside the resolved wall and moves away from it") {
  const auto hit = resolveShot({{0, 1, 0}, {0, 0, -1}},
      {{{-1, 0, -3}, {1, 2, -2}}}, {});
  REQUIRE(hit);
  REQUIRE(hit->normal == glm::vec3(0, 0, 1));
  ShotParticles particles;
  particles.emitImpact(*hit);
  REQUIRE(particles.live().size() == 12);
  for (const auto& particle : particles.live()) {
    REQUIRE(particle.position.z > -2);
    REQUIRE(particle.velocity.z > 0);
  }
  particles.update(0.1F, {{{-1, 0, -3}, {1, 2, -2}}});
  REQUIRE(particles.live().size() == 12);
}

TEST_CASE("Invalid particle input and zero-distance impacts add no effects") {
  ShotParticles particles;
  particles.emitMuzzle({0, 0, 0}, {0, 0, 0});
  particles.emitImpact({ShotHitKind::Solid, 0, 0, {0, 0, 0}, {0, 0, 1}});
  REQUIRE(particles.live().empty());
  particles.emitMuzzle({0, 2, 0}, {0, 0, -1});
  const auto initial = particles.live().front();
  particles.update(-1, {});
  particles.update(std::numeric_limits<float>::quiet_NaN(), {});
  REQUIRE(particles.live().front().position == initial.position);
  REQUIRE(particles.live().front().remaining == initial.remaining);
}
