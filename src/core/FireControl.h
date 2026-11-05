#pragma once

namespace craft::core {

class FireControl {
 public:
  static constexpr float kIntervalSeconds = 0.25F;

  // Consumes a press event after an enabled frame, with a minimum shot interval.
  [[nodiscard]] bool update(float deltaSeconds, bool triggerPressed, bool enabled);

 private:
  float remainingSeconds_ = 0.0F;
  bool wasEnabled_ = false;
};

}
