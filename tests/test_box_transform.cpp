#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <glm/ext/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

#include "core/BoxTransform.h"
#include "core/Camera.h"
#include "core/Shooting.h"

using Catch::Approx;
using namespace craft::core;

namespace {
void requireVector(glm::vec3 actual, glm::vec3 expected) {
  for (int axis = 0; axis < 3; ++axis) {
    REQUIRE(actual[axis] == Approx(expected[axis]).margin(0.00001F));
  }
}
}

TEST_CASE("A translated box keeps its dimensions and moves all corners") {
  const Aabb local{{-1, 0, -2}, {1, 3, 2}};
  const auto world = transformBounds(local,
      glm::translate(glm::mat4(1), {5, 2, -4}));
  requireVector(world.minimum, {4, 2, -6});
  requireVector(world.maximum, {6, 5, -2});
}

TEST_CASE("Negative and nonuniform scales keep transformed bounds ordered") {
  const auto world = transformBounds({{0, 0, 0}, {1, 2, 3}},
      glm::scale(glm::mat4(1), {-2, 3, 0.5F}));
  requireVector(world.minimum, {-2, 0, 0});
  requireVector(world.maximum, {0, 6, 1.5F});
}

TEST_CASE("Rotated hitboxes enclose the full rendered box") {
  const auto world = transformBounds({{-1, 0, -1}, {1, 2, 1}},
      glm::rotate(glm::mat4(1), glm::radians(45.0F), {0, 1, 0}));
  requireVector(world.minimum, {-1.41421356F, 0, -1.41421356F});
  requireVector(world.maximum, {1.41421356F, 2, 1.41421356F});
}

TEST_CASE("The character head transform produces hittable world-space bounds") {
  const glm::mat4 root = glm::scale(
      glm::translate(glm::mat4(1), {11.5F, 5, 15.5F}), glm::vec3(1.0F / 16.0F));
  const auto head = transformBounds({{0, 0, 0}, {8, 8, 8}},
      glm::translate(root, {-4, 24, -4}));
  requireVector(head.minimum, {11.25F, 6.5F, 15.25F});
  requireVector(head.maximum, {11.75F, 7, 15.75F});
  const auto hit = resolveShot({{11.5F, 6.6F, 19.5F}, {0, 0, -1}}, {}, {head});
  REQUIRE(hit);
  REQUIRE(hit->kind == ShotHitKind::Target);
  REQUIRE(hit->distance == Approx(3.75F));
}

TEST_CASE("Moving the character into the hut puts its torso behind the wall") {
  const auto torsoAt = [](glm::vec3 feet) {
    const glm::mat4 root = glm::scale(glm::translate(glm::mat4(1), feet),
                                     glm::vec3(1.0F / 16.0F));
    return transformBounds({{0, 0, 0}, {8, 12, 4}},
                            glm::translate(root, {-4, 12, -2}));
  };
  Camera camera({11.5F, 6.6F, 19.5F});
  camera.look(45.0F, 15.0F);
  const auto inside = torsoAt({15.5F, 5, 15.5F});
  requireVector(inside.minimum, {15.25F, 5.75F, 15.375F});
  requireVector(inside.maximum, {15.75F, 6.5F, 15.625F});
  REQUIRE_FALSE(resolveShot(camera.ray(), {},
                            {torsoAt({11.5F, 5, 15.5F})}));
  const auto unobstructed = resolveShot(camera.ray(), {}, {inside});
  REQUIRE(unobstructed);
  REQUIRE(unobstructed->kind == ShotHitKind::Target);
  const auto blocked = resolveShot(camera.ray(),
      {{{13, 6, 17}, {14, 7, 18}}}, {inside});
  REQUIRE(blocked);
  REQUIRE(blocked->kind == ShotHitKind::Solid);
  REQUIRE(blocked->distance < unobstructed->distance);
}
