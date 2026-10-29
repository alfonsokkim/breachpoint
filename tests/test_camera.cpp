#include "core/Camera.h"

#include <limits>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <glm/geometric.hpp>
#include <glm/vec4.hpp>

using craft::core::Camera;

namespace {
void requireVector(glm::vec3 actual, glm::vec3 expected) {
  for (int axis = 0; axis < 3; ++axis) {
    REQUIRE(actual[axis] == Catch::Approx(expected[axis]).margin(0.0001F));
  }
}
}

TEST_CASE("Camera ray starts at eye height with a unit direction", "[camera]") {
  const Camera camera;
  requireVector(camera.position(), {0, 1.6F, 4});
  requireVector(camera.ray().origin, camera.position());
  requireVector(camera.ray().direction, camera.forward());
  REQUIRE(glm::length(camera.forward()) == Catch::Approx(1.0F));
  REQUIRE(camera.forward().y < 0.0F);
  REQUIRE(camera.forward().z < 0.0F);
}

TEST_CASE("Yaw turns right and positive pitch looks up", "[camera]") {
  Camera camera;
  camera.look(90, 20);
  requireVector(camera.forward(), {1, 0, 0});
  camera.look(90, 45);
  REQUIRE(camera.forward().y > 0.7F);
  REQUIRE(camera.forward().z > 0.7F);
}

TEST_CASE("Camera pitch stays below vertical and yaw wraps", "[camera]") {
  Camera camera;
  camera.look(720, 1000);
  REQUIRE(camera.yawDegrees() == Catch::Approx(-90.0F));
  REQUIRE(camera.pitchDegrees() == 89.0F);
  REQUIRE(glm::length(camera.forward()) == Catch::Approx(1.0F));
  camera.look(0, -1000);
  REQUIRE(camera.pitchDegrees() == -89.0F);
}

TEST_CASE("Walking follows yaw but never changes eye height", "[camera]") {
  Camera camera({0, 1.6F, 0});
  camera.look(0, 100);
  camera.move({0, 1}, 0.5F);
  requireVector(camera.position(), {0, 1.6F, -2});
  camera.move({1, 0}, 0.5F);
  requireVector(camera.position(), {2, 1.6F, -2});
  camera.look(90, 0);
  camera.move({0, 1}, 0.5F);
  requireVector(camera.position(), {4, 1.6F, -2});
}

TEST_CASE("Diagonal walking has the same speed as forward walking", "[camera]") {
  Camera camera({0, 0, 0});
  camera.move({1, 1}, 1.0F);
  REQUIRE(glm::length(camera.position()) == Catch::Approx(4.0F));
  REQUIRE(camera.position().x > 0);
  REQUIRE(camera.position().z < 0);
}

TEST_CASE("Walking distance is independent of frame rate", "[camera]") {
  Camera oneStep;
  Camera sixtySteps;
  oneStep.move({0, 1}, 1.0F);
  for (int frame = 0; frame < 60; ++frame) {
    sixtySteps.move({0, 1}, 1.0F / 60.0F);
  }
  requireVector(oneStep.position(), sixtySteps.position());
}

TEST_CASE("Collision can replace the camera position and ray origin", "[camera]") {
  Camera camera;
  camera.setPosition({2, 3, 4});
  requireVector(camera.position(), {2, 3, 4});
  requireVector(camera.ray().origin, {2, 3, 4});
}

TEST_CASE("The camera ray projects through the centre of the view", "[camera]") {
  Camera camera({2, 3, 4});
  camera.look(25, 10);
  const glm::vec4 point(camera.position() + camera.forward() * 5.0F, 1.0F);
  const glm::vec4 clip = camera.viewProjection(16.0F / 9.0F) * point;
  REQUIRE(clip.w > 0);
  REQUIRE(clip.x / clip.w == Catch::Approx(0.0F).margin(0.0001F));
  REQUIRE(clip.y / clip.w == Catch::Approx(0.0F).margin(0.0001F));
  REQUIRE(clip.z / clip.w > -1);
  REQUIRE(clip.z / clip.w < 1);
  REQUIRE(camera.viewProjection(2)[0][0] ==
          Catch::Approx(camera.viewProjection(1)[0][0] * 0.5F));
}

TEST_CASE("Invalid camera inputs do not corrupt its pose", "[camera]") {
  Camera camera;
  const auto original = camera.position();
  const float invalid = std::numeric_limits<float>::quiet_NaN();
  camera.move({0, 1}, -1);
  camera.move({0, 1}, 0);
  camera.move({invalid, 1}, 1);
  camera.move({0, 1}, invalid);
  camera.look(invalid, 10);
  camera.look(10, invalid);
  camera.setPosition({invalid, 0, 0});
  requireVector(camera.position(), original);
  REQUIRE(camera.yawDegrees() == -90);
  REQUIRE(camera.pitchDegrees() == -20);
  REQUIRE(camera.viewProjection(0) == camera.viewProjection(1));
  REQUIRE(camera.viewProjection(invalid) == camera.viewProjection(1));
}
