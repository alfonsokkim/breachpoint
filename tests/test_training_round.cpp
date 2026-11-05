#include <limits>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "core/TrainingRound.h"
#include "core/SpatialQueries.h"
#include "core/PlayerMovement.h"

using namespace craft::core;
using Catch::Approx;

namespace {
std::vector<Aabb> bounds(const TrainingGround& ground) {
  std::vector<Aabb> result;
  for (const auto& block : ground.blocks) {
    const glm::vec3 p(block.cell);
    result.push_back({p, p + glm::vec3(1)});
  }
  return result;
}
}

TEST_CASE("Both custom courses have supported spawns and reachable targets") {
  for (int level = 0; level < 2; ++level) {
    const auto ground = makeTrainingGround(level);
    const auto solids = bounds(ground);
    REQUIRE(ground.targets.size() >= 5);
    REQUIRE(movePlayer(ground.spawn, ground.spawn + glm::vec3(0, 0, -0.5F), solids).z < ground.spawn.z);
    for (const auto& target : ground.targets) {
      const auto eye = target.feet + glm::vec3(0, 1.6F, 0);
      REQUIRE_FALSE(nearestBoxHit({eye, {0, 1, 0}}, solids, 0));
    }
  }
}

TEST_CASE("Hostiles take two body hits or one headshot and all must fall to win") {
  TrainingRound round;
  round.reset(makeTrainingGround(0), Difficulty::Normal);
  round.hit(0, true);
  REQUIRE(round.targets[0].health == 2);
  round.start();
  round.hit(0, false);
  REQUIRE(round.targets[0].health == 1);
  round.hit(0, false);
  REQUIRE(round.targets[0].health == 0);
  round.hit(0, false);
  REQUIRE(round.hostilesRemaining() == 2);
  round.hit(1, true);
  round.hit(2, true);
  REQUIRE(round.state == RoundState::Won);
}

TEST_CASE("Two separate civilian hits end the round and repeats do not count") {
  TrainingRound round;
  round.reset(makeTrainingGround(0), Difficulty::Normal);
  round.start();
  round.hit(3, true);
  round.hit(3, true);
  REQUIRE(round.civilianStrikes == 1);
  round.hit(4, false);
  REQUIRE(round.state == RoundState::CivilianFailure);
}

TEST_CASE("Counterfire waits for exposure and losing sight resets the delay") {
  auto ground = makeTrainingGround(0);
  ground.targets = {{{0, 0, 0}, false}};
  TrainingRound round;
  round.reset(ground, Difficulty::Normal);
  round.start();
  const glm::vec3 player(0, 1.6F, 5);
  round.update(1.9F, player, {});
  REQUIRE(round.health == 100);
  round.update(0.1F, player, {});
  REQUIRE(round.health == 92);
  REQUIRE(round.targets[0].flash > 0);
  round.update(0.1F, player, {{{-1, 0, 2}, {1, 3, 3}}});
  REQUIRE(round.targets[0].exposure == 0);
  round.update(1.9F, player, {});
  REQUIRE(round.health == 92);
  round.update(0.1F, player, {});
  REQUIRE(round.health == 84);
}

TEST_CASE("Round death timeout and difficulty reset are independent") {
  auto ground = makeTrainingGround(0);
  ground.targets = {{{0, 0, 0}, false}};
  TrainingRound round;
  round.reset(ground, Difficulty::Hard);
  REQUIRE(round.timeRemaining == 60);
  round.start();
  round.update(10, {0, 1.6F, 5}, {});
  REQUIRE(round.state == RoundState::Dead);
  round.reset(ground, Difficulty::Easy);
  REQUIRE(round.health == 100);
  REQUIRE(round.civilianStrikes == 0);
  REQUIRE(round.timeRemaining == 120);
  round.start();
  round.update(121, {0, 1.6F, 5}, {{{-1, 0, 2}, {1, 3, 3}}});
  REQUIRE(round.state == RoundState::Timeout);
  round.update(std::numeric_limits<float>::quiet_NaN(), {}, {});
  REQUIRE(round.timeRemaining == 0);
}

TEST_CASE("Counterfire gives the same result across frame sizes") {
  auto ground = makeTrainingGround(0);
  ground.targets = {{{0, 0, 0}, false}};
  TrainingRound one, many;
  one.reset(ground, Difficulty::Easy);
  many.reset(ground, Difficulty::Easy);
  one.start(); many.start();
  one.update(5.1F, {0, 1.6F, 5}, {});
  for (int i = 0; i < 51; ++i) many.update(0.1F, {0, 1.6F, 5}, {});
  REQUIRE(one.health == many.health);
  REQUIRE(one.timeRemaining == Approx(many.timeRemaining));
}
