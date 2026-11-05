#include "app/AppUi.h"

#include <stdexcept>
#include <algorithm>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "graphics/Window.h"

namespace craft::app {

AppUi::AppUi(graphics::Window& window) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  ImGui::GetStyle().WindowRounding = 0;
  ImGui::GetStyle().FrameRounding = 0;

  if (!ImGui_ImplGlfw_InitForOpenGL(window.nativeHandle(), true)) {
    ImGui::DestroyContext();
    throw std::runtime_error("Failed to initialise Dear ImGui GLFW backend");
  }
  if (!ImGui_ImplOpenGL3_Init("#version 330 core")) {
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    throw std::runtime_error("Failed to initialise Dear ImGui OpenGL backend");
  }
}

AppUi::~AppUi() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void AppUi::beginFrame(bool mouseCaptured) {
  auto& io = ImGui::GetIO();
  if (mouseCaptured) {
    io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
  } else {
    io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
  }
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

bool AppUi::draw(const BenchmarkSnapshot& benchmark) {
  if (ImGui::IsKeyPressed(ImGuiKey_H, false) || ImGui::IsKeyPressed(ImGuiKey_F3, false))
    showControls_ = !showControls_;
  customUi_.drawHud(benchmark);
  if (!showControls_) return false;
  const auto size = ImGui::GetIO().DisplaySize;
  ImGui::SetNextWindowSize(ImVec2(std::min(320.0F, size.x - 32.0F),
                                 std::min(560.0F, size.y - 32.0F)), ImGuiCond_Always);
  ImGui::SetNextWindowPos(ImVec2(std::max(16.0F, size.x - 336.0F), 16.0F), ImGuiCond_Always);
  bool startBenchmark = false;
  if (ImGui::Begin("SETTINGS / H TO CLOSE", &showControls_, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
    customUi_.drawContents(benchmark.running);
    ImGui::SeparatorText("240-frame benchmark");
    startBenchmark = drawBenchmarkSection(benchmark);
  }
  ImGui::End();
  return startBenchmark;
}

void AppUi::render() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void AppUi::setDiagnostics(const CameraDiagnostics& diagnostics) {
  customUi_.setDiagnostics(diagnostics);
}

bool AppUi::wantsKeyboardInput() const {
  return ImGui::GetIO().WantCaptureKeyboard;
}

bool AppUi::drawBenchmarkSection(const BenchmarkSnapshot& benchmark) const {
  bool startBenchmark = false;

  if (benchmark.running) {
    if (benchmark.warmupFrames < benchmark.requiredWarmupFrames) {
      ImGui::Text("Warm-up: %zu / %zu", benchmark.warmupFrames,
                  benchmark.requiredWarmupFrames);
    } else {
      ImGui::Text("Measured: %zu / %zu", benchmark.measuredFrames,
                  benchmark.requiredMeasuredFrames);
      ImGui::Text("Average: %.4f ms/frame", benchmark.averageMilliseconds);
      ImGui::Text("Average FPS: %.1f", benchmark.averageFramesPerSecond);
    }
  } else {
    if (benchmark.complete) {
      ImGui::Text("Complete: %zu samples", benchmark.measuredFrames);
      ImGui::Text("Average: %.4f ms/frame", benchmark.averageMilliseconds);
      ImGui::Text("Average FPS: %.1f", benchmark.averageFramesPerSecond);
    } else {
      ImGui::TextUnformatted("Ready");
    }
    startBenchmark = ImGui::Button("Run benchmark");
  }

  return startBenchmark;
}

}  // namespace craft::app
