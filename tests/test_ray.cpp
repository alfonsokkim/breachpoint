#include "core/Ray.h"

#include <limits>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using craft::core::Aabb;
using craft::core::intersectRayAabb;
using craft::core::Ray;

namespace {
const Aabb box{{1.0F, 1.0F, 1.0F}, {2.0F, 2.0F, 2.0F}};
}

TEST_CASE("Rays enter boxes from either direction on every axis", "[ray]") {
  for (int axis = 0; axis < 3; ++axis) {
    Ray ray{glm::vec3(1.5F), glm::vec3(0.0F)};
    ray.origin[axis] = 0.0F;
    ray.direction[axis] = 1.0F;
    auto hit = intersectRayAabb(ray, box);
    REQUIRE(hit.has_value());
    REQUIRE(*hit == Catch::Approx(1.0F));

    ray.origin[axis] = 3.0F;
    ray.direction[axis] = -1.0F;
    hit = intersectRayAabb(ray, box);
    REQUIRE(hit.has_value());
    REQUIRE(*hit == Catch::Approx(1.0F));
  }
}

TEST_CASE("A hit needs overlapping intervals on all three axes", "[ray]") {
  REQUIRE_FALSE(intersectRayAabb({{0, 0, 1.5F}, {1, 3, 0}}, box));
  const auto hit = intersectRayAabb({{0, 0, 0}, {1, 1, 1}}, box);
  REQUIRE(hit.has_value());
  REQUIRE(*hit == Catch::Approx(1.0F));
}

TEST_CASE("Parallel rays miss when outside a box slab", "[ray]") {
  REQUIRE_FALSE(intersectRayAabb({{0, 0, 1.5F}, {1, 0, 0}}, box));
  REQUIRE_FALSE(intersectRayAabb({{0, 1.5F, 3}, {1, 0, 0}}, box));
}

TEST_CASE("Boxes behind the ray origin are ignored", "[ray]") {
  REQUIRE_FALSE(intersectRayAabb({{0, 1.5F, 1.5F}, {-1, 0, 0}}, box));
}

TEST_CASE("Starting inside or on the box gives an immediate hit", "[ray]") {
  for (const float x : {1.0F, 1.5F, 2.0F}) {
    const auto hit = intersectRayAabb({{x, 1.5F, 1.5F}, {1, 0, 0}}, box);
    REQUIRE(hit.has_value());
    REQUIRE(*hit == 0.0F);
  }
}

TEST_CASE("Touching a face edge or corner counts as a hit", "[ray]") {
  for (const glm::vec3 origin : {glm::vec3{0, 1, 1.5F}, glm::vec3{0, 1, 1}}) {
    const auto hit = intersectRayAabb({origin, {1, 0, 0}}, box);
    REQUIRE(hit.has_value());
    REQUIRE(*hit == Catch::Approx(1.0F));
  }
  const auto hit = intersectRayAabb({{0, 2, 1.5F}, {1, -1, 0}}, box);
  REQUIRE(hit.has_value());
  REQUIRE(*hit == Catch::Approx(1.0F));
}

TEST_CASE("Ray limits include their endpoint", "[ray]") {
  const Ray ray{{0, 1.5F, 1.5F}, {1, 0, 0}};
  REQUIRE_FALSE(intersectRayAabb(ray, box, 0.99F));
  REQUIRE(intersectRayAabb(ray, box, 1.0F).has_value());
  REQUIRE(intersectRayAabb({{1.5F, 1.5F, 1.5F}, {1, 0, 0}}, box, 0.0F));
}

TEST_CASE("Direction length scales the ray parameter", "[ray]") {
  const auto hit = intersectRayAabb({{0, 1.5F, 1.5F}, {2, 0, 0}}, box);
  REQUIRE(hit.has_value());
  REQUIRE(*hit == Catch::Approx(0.5F));
}

TEST_CASE("Small nonzero directions still intersect distant boxes", "[ray]") {
  const auto hit = intersectRayAabb({{0, 1.5F, 1.5F}, {1e-8F, 0, 0}}, box);
  REQUIRE(hit.has_value());
  REQUIRE(*hit == Catch::Approx(1e8F));
}

TEST_CASE("Invalid rays bounds and limits return no hit", "[ray]") {
  const Ray ray{{0, 1.5F, 1.5F}, {1, 0, 0}};
  REQUIRE_FALSE(intersectRayAabb({{1.5F, 1.5F, 1.5F}, {0, 0, 0}}, box));
  REQUIRE_FALSE(intersectRayAabb(ray, {{2, 1, 1}, {1, 2, 2}}));
  REQUIRE_FALSE(intersectRayAabb(ray, box, -1.0F));
  for (const float invalid : {std::numeric_limits<float>::infinity(),
                              std::numeric_limits<float>::quiet_NaN()}) {
    REQUIRE_FALSE(intersectRayAabb(ray, box, invalid));
    REQUIRE_FALSE(intersectRayAabb({{invalid, 1.5F, 1.5F}, {1, 0, 0}}, box));
    REQUIRE_FALSE(intersectRayAabb({ray.origin, {invalid, 0, 0}}, box));
    REQUIRE_FALSE(intersectRayAabb(ray, {{invalid, 1, 1}, {2, 2, 2}}));
  }
}
