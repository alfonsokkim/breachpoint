#include "core/TrainingRound.h"

#include <algorithm>
#include <cmath>

#include "core/SpatialQueries.h"

namespace craft::core {

void TrainingRound::reset(const TrainingGround& ground, Difficulty setting) {
  *this = TrainingRound{};
  difficulty = setting;
  timeRemaining = setting == Difficulty::Easy ? 120 : setting == Difficulty::Hard ? 60 : 90;
  for (const auto& spawn : ground.targets) targets.push_back({spawn});
}

void TrainingRound::start() {
  if (state == RoundState::Ready) state = RoundState::Playing;
}

float TrainingRound::exposureDelay() const {
  return difficulty == Difficulty::Easy ? 3.0F : difficulty == Difficulty::Hard ? 1.25F : 2.0F;
}
float TrainingRound::damageInterval() const { return difficulty == Difficulty::Hard ? 0.7F : 1.0F; }
int TrainingRound::pulseDamage() const { return difficulty == Difficulty::Easy ? 5 : difficulty == Difficulty::Hard ? 12 : 8; }

int TrainingRound::hostilesRemaining() const {
  return static_cast<int>(std::count_if(targets.begin(), targets.end(),
      [](const TargetState& t) { return !t.spawn.civilian && t.health > 0; }));
}

void TrainingRound::hit(std::size_t target, bool headshot) {
  if (state != RoundState::Playing || target >= targets.size()) return;
  auto& t = targets[target];
  if (t.health <= 0) return;
  if (t.spawn.civilian) {
    ++civilianStrikes;
    t.health = 0;
    if (civilianStrikes >= 2) state = RoundState::CivilianFailure;
  } else {
    t.health = std::max(0, t.health - (headshot ? 2 : 1));
    if (hostilesRemaining() == 0) state = RoundState::Won;
  }
  if (t.health <= 0) { t.exposure = 0; t.flash = 0; }
}

void TrainingRound::update(float seconds, glm::vec3 player, const std::vector<Aabb>& solids) {
  if (!std::isfinite(seconds) || seconds <= 0) return;
  for (auto& t : targets) {
    t.flash = std::max(0.0F, t.flash - seconds);
    if (t.health <= 0) t.fall = std::min(1.0F, t.fall + seconds * 3.0F);
  }
  if (state != RoundState::Playing) return;
  const float activeSeconds = std::min(seconds, timeRemaining);
  timeRemaining = std::max(0.0F, timeRemaining - seconds);
  for (auto& t : targets) {
    if (t.health <= 0 || t.spawn.civilian) continue;
    const glm::vec3 eye = t.spawn.feet + glm::vec3(0, 1.55F, 0);
    if (!hasLineOfSight(eye, player, solids)) {
      t.exposure = 0;
      t.nextPulse = 0;
      continue;
    }
    const float previous = t.exposure;
    t.exposure += activeSeconds;
    if (previous < exposureDelay() && t.exposure >= exposureDelay()) t.nextPulse = exposureDelay();
    while (t.exposure >= exposureDelay() && t.exposure >= t.nextPulse) {
      health = std::max(0, health - pulseDamage());
      t.flash = 0.12F;
      t.nextPulse += damageInterval();
      if (health == 0) { state = RoundState::Dead; return; }
    }
  }
  if (timeRemaining == 0) state = RoundState::Timeout;
}

const char* roundStateName(RoundState state) {
  switch (state) {
    case RoundState::Ready: return "READY";
    case RoundState::Playing: return "LIVE";
    case RoundState::Won: return "COURSE CLEAR";
    case RoundState::Dead: return "OPERATOR DOWN";
    case RoundState::Timeout: return "TIME EXPIRED";
    case RoundState::CivilianFailure: return "CIVILIAN LIMIT";
  }
  return "";
}

}
