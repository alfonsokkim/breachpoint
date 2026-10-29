#include "core/SpatialQueries.h"

#include <algorithm>
#include <limits>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using craft::core::Aabb;
using craft::core::hasLineOfSight;
using craft::core::nearestBoxHit;
using craft::core::Ray;

namespace {
const Aabb nearBox{{1, -1, -1}, {2, 1, 1}};
const Aabb farBox{{3, -1, -1}, {4, 1, 1}};
const Ray forward{{0, 0, 0}, {1, 0, 0}};
}

TEST_CASE("Nearest hit returns the closest box regardless of order", "[query]") {
  std::vector<Aabb> boxes{farBox, nearBox};
  auto hit = nearestBoxHit(forward, boxes);
  REQUIRE(hit.has_value());
  REQUIRE(hit->index == 1);
  REQUIRE(hit->t == Catch::Approx(1));
  std::reverse(boxes.begin(), boxes.end());
  hit = nearestBoxHit(forward, boxes);
  REQUIRE(hit.has_value());
  REQUIRE(hit->index == 0);
  REQUIRE(hit->t == Catch::Approx(1));
}

TEST_CASE("Equal hits keep the first box and inside hits return zero", "[query]") {
  auto hit = nearestBoxHit(forward, {nearBox, nearBox});
  REQUIRE(hit.has_value());
  REQUIRE(hit->index == 0);
  hit = nearestBoxHit({{1.5F, 0, 0}, {1, 0, 0}}, {farBox, nearBox});
  REQUIRE(hit.has_value());
  REQUIRE(hit->index == 1);
  REQUIRE(hit->t == 0);
}

TEST_CASE("Nearest hit respects its inclusive ray parameter limit", "[query]") {
  REQUIRE_FALSE(nearestBoxHit(forward, {nearBox}, 0.99F));
  REQUIRE(nearestBoxHit(forward, {nearBox}, 1.0F));
  const auto hit = nearestBoxHit({{0, 0, 0}, {2, 0, 0}}, {nearBox});
  REQUIRE(hit.has_value());
  REQUIRE(hit->t == Catch::Approx(0.5F));
}

TEST_CASE("Empty or missed boxes produce no nearest hit", "[query]") {
  REQUIRE_FALSE(nearestBoxHit(forward, {}));
  REQUIRE_FALSE(nearestBoxHit({{0, 2, 0}, {1, 0, 0}}, {nearBox}));
  REQUIRE_FALSE(nearestBoxHit(forward, {{{2, 1, 1}, {1, -1, -1}}}));
  REQUIRE_FALSE(nearestBoxHit(forward, {nearBox}, -1));
}

TEST_CASE("Line of sight is blocked only along the bounded segment", "[query]") {
  REQUIRE_FALSE(hasLineOfSight({0, 0, 0}, {5, 0, 0}, {nearBox}));
  REQUIRE_FALSE(hasLineOfSight({5, 0, 0}, {0, 0, 0}, {nearBox}));
  REQUIRE(hasLineOfSight({0, 0, 0}, {0.5F, 0, 0}, {nearBox}));
  REQUIRE(hasLineOfSight({2.5F, 0, 0}, {5, 0, 0}, {nearBox}));
  REQUIRE(hasLineOfSight({0, 2, 0}, {5, 2, 0}, {nearBox}));
  REQUIRE(hasLineOfSight({0, 0, 0}, {5, 0, 0}, {}));
}

TEST_CASE("Line of sight counts touching surfaces as blocked", "[query]") {
  REQUIRE_FALSE(hasLineOfSight({0, 0, 0}, {1, 0, 0}, {nearBox}));
  REQUIRE_FALSE(hasLineOfSight({2, 0, 0}, {5, 0, 0}, {nearBox}));
  REQUIRE_FALSE(hasLineOfSight({0, 1, 1}, {5, 1, 1}, {nearBox}));
  REQUIRE_FALSE(hasLineOfSight({1.5F, 0, 0}, {5, 0, 0}, {nearBox}));
}

TEST_CASE("A zero length sight segment is a point overlap check", "[query]") {
  REQUIRE(hasLineOfSight({0, 0, 0}, {0, 0, 0}, {nearBox}));
  REQUIRE_FALSE(hasLineOfSight({1.5F, 0, 0}, {1.5F, 0, 0}, {nearBox}));
  REQUIRE_FALSE(hasLineOfSight({1, 1, 1}, {1, 1, 1}, {nearBox}));
}

TEST_CASE("Nonfinite sight endpoints cannot report a clear path", "[query]") {
  for (const float invalid : {std::numeric_limits<float>::infinity(),
                              std::numeric_limits<float>::quiet_NaN()}) {
    REQUIRE_FALSE(hasLineOfSight({invalid, 0, 0}, {5, 0, 0}, {}));
    REQUIRE_FALSE(hasLineOfSight({0, 0, 0}, {invalid, 0, 0}, {}));
  }
}
