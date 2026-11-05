#pragma once

#include <optional>

#include <glm/vec3.hpp>

#include "core/Shooting.h"

namespace craft::app {

struct CameraDiagnostics {
  glm::vec3 position{};
  glm::vec3 forward{};
  float yaw = 0.0F;
  float pitch = 0.0F;
  bool mouseCaptured = false;
  std::optional<float> solidHitDistance;
  int worldFaces = 0;
  std::optional<core::ShotHit> lastShotHit;
  unsigned int shotsFired = 0;
  float feedbackSeconds = 0.0F;
};

// Student-facing extension point for assignment-specific Dear ImGui controls.
// It deliberately has no access to SceneBenchmark internals.
class CustomUi {
 public:
  // Draws only the custom component inside an existing ImGui window.
  void drawContents(bool interactionDisabled);

  void setDiagnostics(const CameraDiagnostics& diagnostics);

 private:
  CameraDiagnostics diagnostics_;
};

}  // namespace craft::app
