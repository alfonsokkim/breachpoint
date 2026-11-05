#include <limits>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <glm/geometric.hpp>
#include <glm/vec4.hpp>

#include "core/MuzzleFlash.h"
#include "core/Camera.h"

using namespace craft::core;
using Catch::Approx;

TEST_CASE("Muzzle light fades with elapsed time and resets between demos") {
  MuzzleFlash flash;
  REQUIRE(flash.intensity() == 0);
  flash.trigger();
  REQUIRE(flash.intensity() == 1);
  flash.update(0.06F);
  REQUIRE(flash.intensity() == Approx(0.5F));
  flash.update(-1);
  flash.update(std::numeric_limits<float>::quiet_NaN());
  REQUIRE(flash.intensity() == Approx(0.5F));
  flash.update(1);
  REQUIRE(flash.intensity() == 0);
  flash.trigger();
  flash.reset();
  REQUIRE(flash.intensity() == 0);
}

TEST_CASE("The weapon frame follows aim without reflecting the mesh") {
  const Ray camera{{11.5F, 6.6F, 19.5F}, {0, 0, -1}};
  const auto frame = weaponFrame(camera);
  REQUIRE(glm::dot(glm::cross(glm::vec3(frame[0]), glm::vec3(frame[1])),
                    glm::vec3(frame[2])) == Approx(1));
  const glm::vec3 mouth = muzzlePosition(camera, {});
  REQUIRE(mouth.x == Approx(11.7F));
  REQUIRE(mouth.y == Approx(6.45F));
  REQUIRE(mouth.z == Approx(18.75F));
  const auto vertical = weaponFrame({{0, 0, 0}, {0, 1, 0}});
  REQUIRE(glm::length(glm::vec3(vertical[0])) == Approx(1));
}

TEST_CASE("A nearby wall keeps the muzzle light on the player's side") {
  const Ray camera{{0, 1.6F, 0}, {0, 0, -1}};
  const Aabb wall{{-1, 0, -0.5F}, {1, 3, -0.3F}};
  const auto mouth = muzzlePosition(camera, {wall});
  REQUIRE(mouth.z > -0.3F);
  REQUIRE(mouth.z < 0);
  REQUIRE_FALSE(nearestBoxHit({camera.origin, mouth - camera.origin}, {wall}, 1));
}

TEST_CASE("The gun stays on the same side throughout supported vertical aim") {
  for (float yaw : {-90.0F, 0.0F, 60.0F}) {
    Camera level;
    level.look(yaw + 90.0F, 20.0F);
    const glm::vec3 expectedRight(weaponFrame(level.ray())[0]);
    for (float pitch : {-89.0F, -88.0F, -87.0F, 87.0F, 88.0F, 89.0F}) {
      Camera camera;
      camera.look(yaw + 90.0F, pitch + 20.0F);
      const glm::vec3 right(weaponFrame(camera.ray())[0]);
      REQUIRE(glm::dot(right, expectedRight) == Approx(1));
    }
  }
}
