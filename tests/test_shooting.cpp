#include <limits>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "core/Shooting.h"

using Catch::Approx;
using namespace craft::core;

namespace {
const Ray kRay{{0, 0, 0}, {0, 0, -1}};
Aabb boxAt(float distance) {
  return {{-0.5F, -0.5F, -distance - 1.0F},
           {0.5F, 0.5F, -distance}};
}
}

TEST_CASE("Shots stop at the first wall instead of hitting a target behind it") {
  const auto hit = resolveShot(kRay, {boxAt(2)}, {boxAt(4)});
  REQUIRE(hit);
  REQUIRE(hit->kind == ShotHitKind::Solid);
  REQUIRE(hit->distance == Approx(2));
  REQUIRE(hit->position == glm::vec3(0, 0, -2));
}

TEST_CASE("Only the nearest visible target is hit regardless of storage order") {
  const auto hit = resolveShot(kRay, {boxAt(6)}, {boxAt(4), boxAt(2)});
  REQUIRE(hit);
  REQUIRE(hit->kind == ShotHitKind::Target);
  REQUIRE(hit->index == 1);
  REQUIRE(hit->distance == Approx(2));
}

TEST_CASE("A wall sharing the target hit distance blocks the shot") {
  const auto hit = resolveShot(kRay, {boxAt(2)}, {boxAt(2)});
  REQUIRE(hit);
  REQUIRE(hit->kind == ShotHitKind::Solid);
}

TEST_CASE("Shot range is an inclusive world distance even for nonunit rays") {
  for (float scale : {0.1F, 1.0F, 10.0F}) {
    const Ray ray{kRay.origin, kRay.direction * scale};
    REQUIRE_FALSE(resolveShot(ray, {}, {boxAt(5)}, 4.99F));
    const auto hit = resolveShot(ray, {}, {boxAt(5)}, 5.0F);
    REQUIRE(hit);
    REQUIRE(hit->distance == Approx(5));
  }
}

TEST_CASE("A shot starting inside a solid cannot hit a target outside it") {
  const auto hit = resolveShot(kRay, {{{-1, -1, -1}, {1, 1, 1}}}, {boxAt(2)});
  REQUIRE(hit);
  REQUIRE(hit->kind == ShotHitKind::Solid);
  REQUIRE(hit->distance == 0.0F);
}

TEST_CASE("Shots miss empty space and boxes away from the aim ray") {
  REQUIRE_FALSE(resolveShot(kRay, {}, {}));
  REQUIRE_FALSE(resolveShot(kRay, {}, {{{2, -1, -3}, {3, 1, -2}}}));
  REQUIRE_FALSE(resolveShot(kRay, {}, {boxAt(-3)}));
}

TEST_CASE("Invalid shot inputs do not produce hits") {
  const float nan = std::numeric_limits<float>::quiet_NaN();
  const float inf = std::numeric_limits<float>::infinity();
  REQUIRE_FALSE(resolveShot({{0, 0, 0}, {0, 0, 0}}, {}, {boxAt(2)}));
  REQUIRE_FALSE(resolveShot({{0, 0, 0}, {nan, 0, -1}}, {}, {boxAt(2)}));
  REQUIRE_FALSE(resolveShot({{inf, 0, 0}, {0, 0, -1}}, {}, {boxAt(2)}));
  for (float range : {-1.0F, nan, inf}) {
    REQUIRE_FALSE(resolveShot(kRay, {}, {boxAt(2)}, range));
  }
}

TEST_CASE("Resolved normals face out of the entry surface on every axis") {
  const Aabb box{{-1, -1, -1}, {1, 1, 1}};
  for (int axis = 0; axis < 3; ++axis) {
    for (float sign : {-1.0F, 1.0F}) {
      glm::vec3 origin(0), direction(0), expected(0);
      origin[axis] = sign * 3;
      direction[axis] = -sign;
      expected[axis] = sign;
      const auto hit = resolveShot({origin, direction}, {box}, {});
      REQUIRE(hit);
      REQUIRE(hit->normal == expected);
    }
  }
}
