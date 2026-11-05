#pragma once

#include <optional>

#include <glm/vec3.hpp>

#include "core/Shooting.h"
#include "core/TrainingRound.h"

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
  bool targetInsideHut = false;
  bool lastShotObstructed = false;
  int effectPoints = 0;
  const core::TrainingRound* round = nullptr;
  const char* levelName = "";
  int sceneTriangles = 0;
  int sceneDraws = 0;
};

struct RenderSettings {
  bool nightVision = false;
  bool brdf = true;
  float roughness = 0.3F;
  float metallic = 0.75F;
};

// Student-facing extension point for assignment-specific Dear ImGui controls.
// It deliberately has no access to SceneBenchmark internals.
class CustomUi {
 public:
  // Draws only the custom component inside an existing ImGui window.
  void drawContents(bool interactionDisabled);

  void setDiagnostics(const CameraDiagnostics& diagnostics);
  RenderSettings& settings() { return settings_; }

 private:
  CameraDiagnostics diagnostics_;
  RenderSettings settings_;
};

}  // namespace craft::app
