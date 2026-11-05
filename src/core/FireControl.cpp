#include "core/FireControl.h"

#include <algorithm>
#include <cmath>

namespace craft::core {

bool FireControl::update(float deltaSeconds, bool triggerPressed, bool enabled) {
  const bool canFire = enabled && wasEnabled_;
  wasEnabled_ = enabled;
  if (!std::isfinite(deltaSeconds) || deltaSeconds < 0.0F) {
    return false;
  }

  remainingSeconds_ = std::max(0.0F, remainingSeconds_ - deltaSeconds);
  if (!canFire || !triggerPressed || remainingSeconds_ > 0.0F) {
    return false;
  }
  remainingSeconds_ = kIntervalSeconds;
  return true;
}

}
