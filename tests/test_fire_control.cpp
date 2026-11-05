#include <limits>

#include <catch2/catch_test_macros.hpp>

#include "core/FireControl.h"

using craft::core::FireControl;

TEST_CASE("A press event fires once and holding generates no repeat shots") {
  FireControl fire;
  REQUIRE_FALSE(fire.update(0.0F, false, true));
  REQUIRE(fire.update(0.0F, true, true));
  REQUIRE_FALSE(fire.update(1.0F, false, true));
  REQUIRE_FALSE(fire.update(0.0F, false, true));
  REQUIRE(fire.update(0.0F, true, true));
}

TEST_CASE("Fast presses are discarded until the firing cooldown expires") {
  FireControl fire;
  REQUIRE_FALSE(fire.update(0.0F, false, true));
  REQUIRE(fire.update(0.0F, true, true));
  REQUIRE_FALSE(fire.update(0.125F, false, true));
  REQUIRE_FALSE(fire.update(0.0F, true, true));
  REQUIRE_FALSE(fire.update(0.125F, false, true));
  REQUIRE_FALSE(fire.update(0.0F, false, true));
  REQUIRE(fire.update(0.0F, true, true));
}

TEST_CASE("Separate press events need no release frame between them") {
  FireControl fire;
  REQUIRE_FALSE(fire.update(0.0F, false, true));
  REQUIRE(fire.update(0.0F, true, true));
  REQUIRE(fire.update(FireControl::kIntervalSeconds, true, true));
}

TEST_CASE("Enabling shooting or returning with a held trigger does not fire") {
  FireControl fire;
  REQUIRE_FALSE(fire.update(1.0F, true, false));
  REQUIRE_FALSE(fire.update(1.0F, false, true));
  REQUIRE_FALSE(fire.update(0.0F, false, true));
  REQUIRE(fire.update(0.0F, true, true));
  REQUIRE_FALSE(fire.update(1.0F, false, false));
  REQUIRE_FALSE(fire.update(0.0F, true, true));
  REQUIRE_FALSE(fire.update(1.0F, false, true));
  REQUIRE_FALSE(fire.update(0.0F, false, true));
  REQUIRE(fire.update(0.0F, true, true));
}

TEST_CASE("Invalid frame times neither fire nor bypass a cooldown") {
  FireControl fire;
  REQUIRE_FALSE(fire.update(0.0F, false, true));
  REQUIRE(fire.update(0.0F, true, true));
  for (float delta : {-1.0F, std::numeric_limits<float>::infinity(),
                      std::numeric_limits<float>::quiet_NaN()}) {
    REQUIRE_FALSE(fire.update(0.0F, false, true));
    REQUIRE_FALSE(fire.update(delta, true, true));
  }
  REQUIRE_FALSE(fire.update(0.0F, false, true));
  REQUIRE_FALSE(fire.update(0.0F, true, true));
  REQUIRE_FALSE(fire.update(FireControl::kIntervalSeconds, false, true));
  REQUIRE(fire.update(0.0F, true, true));
}
