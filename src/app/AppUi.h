#pragma once

#include "app/CustomUi.h"
#include "app/SceneBenchmark.h"

namespace craft::graphics {
class Window;
}

namespace craft::app {

// Owns the Dear ImGui lifecycle and composes one compact control window.
// Assignment-specific controls remain isolated in CustomUi.
class AppUi {
 public:
  explicit AppUi(graphics::Window& window);
  ~AppUi();

  AppUi(const AppUi&) = delete;
  AppUi& operator=(const AppUi&) = delete;

  void beginFrame(bool mouseCaptured);
  // Returns true when the course-provided benchmark section requests a run.
  [[nodiscard]] bool draw(const BenchmarkSnapshot& benchmark);
  void render();

  void setDiagnostics(const CameraDiagnostics& diagnostics);
  [[nodiscard]] bool wantsKeyboardInput() const;

 private:
  [[nodiscard]] bool drawBenchmarkSection(
      const BenchmarkSnapshot& benchmark) const;

  CustomUi customUi_;
};

}  // namespace craft::app
