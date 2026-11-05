#pragma once

#include <vector>

#include "core/TrainingGround.h"

namespace craft::core {

enum class RoundState { Ready, Playing, Won, Dead, Timeout, CivilianFailure };
enum class Difficulty { Easy, Normal, Hard };
struct TargetState {
  TargetSpawn spawn;
  int health = 2;
  float fall = 0;
  float exposure = 0;
  float nextPulse = 0;
  float flash = 0;
};

// Owns the round rules; rendering and input stay outside this class.
class TrainingRound {
 public:
  void reset(const TrainingGround& ground, Difficulty difficulty);
  void start();
  void hit(std::size_t target, bool headshot);
  void update(float seconds, glm::vec3 player, const std::vector<Aabb>& solids);
  [[nodiscard]] int hostilesRemaining() const;
  [[nodiscard]] float exposureDelay() const;
  [[nodiscard]] float damageInterval() const;
  [[nodiscard]] int pulseDamage() const;

  RoundState state = RoundState::Ready;
  Difficulty difficulty = Difficulty::Normal;
  float timeRemaining = 90;
  int health = 100;
  int civilianStrikes = 0;
  int shots = 0;
  std::vector<TargetState> targets;
};

[[nodiscard]] const char* roundStateName(RoundState state);

}
