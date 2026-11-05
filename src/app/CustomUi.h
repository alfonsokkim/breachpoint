#pragma once

#include <optional>

#include <glm/vec3.hpp>

#include "core/Shooting.h"
#include "core/TrainingRound.h"
#include "app/SceneBenchmark.h"

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
  bool lastShotCivilian = false;
};

struct RenderSettings {
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
  void drawHud(const BenchmarkSnapshot& benchmark);

  void setDiagnostics(const CameraDiagnostics& diagnostics);
  RenderSettings& settings() { return settings_; }

 private:
  CameraDiagnostics diagnostics_;
  RenderSettings settings_;
  int previousHealth_ = 100;
  int previousStrikes_ = 0;
  double damageUntil_ = 0;
  double civilianUntil_ = 0;
};

}  // namespace craft::app
