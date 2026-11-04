#include <limits>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "core/Camera.h"
#include "core/PlayerMovement.h"

using Catch::Approx;
using namespace craft::core;

namespace {
const Aabb kFloor{{0, 4, 0}, {12, 5, 12}};
const glm::vec3 kEye{2, 6.6F, 2};
}

TEST_CASE("Walking stays at eye height above a raised floor") {
  const auto moved = movePlayer(kEye, {3, 20, 4}, {kFloor});
  REQUIRE(moved == glm::vec3(3, kEye.y, 4));
  REQUIRE(playerBounds(moved).minimum.y == Approx(5));
  REQUIRE(playerBounds(moved).maximum.y == Approx(6.8F));
}

TEST_CASE("A player stops against walls in both directions without tunnelling") {
  const std::vector<Aabb> solids{kFloor, {{5, 5, 0}, {6, 8, 12}}};
  REQUIRE(movePlayer(kEye, {10, kEye.y, 2}, solids).x == Approx(4.75F));
  REQUIRE(movePlayer({9, kEye.y, 2}, kEye, solids).x == Approx(6.25F));
  REQUIRE(movePlayer({2, kEye.y, 4}, {2, kEye.y, 10},
      {kFloor, {{0, 5, 6}, {12, 8, 7}}}).z == Approx(5.75F));
  REQUIRE(movePlayer({2, kEye.y, 10}, kEye,
      {kFloor, {{0, 5, 6}, {12, 8, 7}}}).z == Approx(7.25F));
}

TEST_CASE("Diagonal movement slides along a wall") {
  const auto moved = movePlayer(kEye, {8, kEye.y, 8},
      {kFloor, {{5, 5, 0}, {6, 8, 12}}});
  REQUIRE(moved.x == Approx(4.75F));
  REQUIRE(moved.z == Approx(8));
}

TEST_CASE("The player fits through a one-block doorway beneath a low roof") {
  const std::vector<Aabb> solids{kFloor,
      {{5, 5, 0}, {6, 7, 5}}, {{5, 5, 6}, {6, 7, 12}},
      {{5, 7, 0}, {6, 8, 12}}};
  REQUIRE(movePlayer({2, kEye.y, 5.5F}, {8, kEye.y, 5.5F}, solids).x == Approx(8));
  REQUIRE(movePlayer({2, kEye.y, 5.1F}, {8, kEye.y, 5.1F}, solids).x == Approx(4.75F));
}

TEST_CASE("Walking stops at each platform edge with the body supported") {
  REQUIRE(movePlayer(kEye, {-10, kEye.y, 2}, {kFloor}).x == Approx(0.25F));
  REQUIRE(movePlayer(kEye, {20, kEye.y, 2}, {kFloor}).x == Approx(11.75F));
  REQUIRE(movePlayer(kEye, {2, kEye.y, -10}, {kFloor}).z == Approx(0.25F));
  REQUIRE(movePlayer(kEye, {2, kEye.y, 20}, {kFloor}).z == Approx(11.75F));
}

TEST_CASE("Ground support crosses adjacent blocks but never jumps a gap") {
  const std::vector<Aabb> floor{
      {{0, 4, 0}, {3, 5, 5}}, {{3, 4, 0}, {6, 5, 5}},
      {{7, 4, 0}, {12, 5, 5}}};
  REQUIRE(movePlayer(kEye, {5, kEye.y, 2}, floor).x == Approx(5));
  REQUIRE(movePlayer(kEye, {10, kEye.y, 2}, floor).x == Approx(5.75F));
  REQUIRE(movePlayer({9, kEye.y, 2}, kEye, floor).x == Approx(7.25F));
}

TEST_CASE("Collision is independent of solid-box ordering") {
  const Aabb wall{{5, 5, 0}, {6, 8, 12}};
  REQUIRE(movePlayer(kEye, {10, kEye.y, 8}, {kFloor, wall}) ==
          movePlayer(kEye, {10, kEye.y, 8}, {wall, kFloor}));
}

TEST_CASE("Camera movement connects to collision without changing the aim ray") {
  Camera camera(kEye);
  camera.look(90, 20);
  camera.move({0, 1}, 1.0F);
  camera.setPosition(movePlayer(kEye, camera.position(),
      {kFloor, {{5, 5, 0}, {6, 8, 12}}}));
  REQUIRE(camera.position().x == Approx(4.75F));
  REQUIRE(camera.ray().origin == camera.position());
  REQUIRE(camera.forward().x == Approx(1));
}

TEST_CASE("Unsupported or invalid movement leaves the player in place") {
  REQUIRE(movePlayer(kEye, {3, kEye.y, 2}, {}) == kEye);
  REQUIRE(movePlayer(kEye, {3, kEye.y, 2}, {{{0, 5, 0}, {5, 8, 5}}}) == kEye);
  REQUIRE(movePlayer(kEye, {std::numeric_limits<float>::quiet_NaN(), 0, 0},
                     {kFloor}) == kEye);
}
