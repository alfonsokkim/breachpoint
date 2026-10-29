#include <algorithm>
#include <iomanip>
#include <iostream>
#include <optional>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/vec3.hpp>

#include "app/AppUi.h"
#include "app/CameraController.h"
#include "app/SceneBenchmark.h"
#include "app/TitleScreen.h"
#include "core/AssetPath.h"
#include "core/Camera.h"
#include "core/SpatialQueries.h"
#include "graphics/Example3DMesh.h"
#include "graphics/OpenGL.h"
#include "graphics/Shader.h"
#include "graphics/Window.h"

namespace {

constexpr int kInitialWidth = 1280;
constexpr int kInitialHeight = 720;
constexpr const char* kStarterWorkload = "starter-3d-tetrahedron";

void startBenchmark(craft::graphics::Window& window,
                    craft::app::SceneBenchmark& benchmark,
                    craft::core::Camera& camera,
                    craft::app::CameraController& controller) {
  controller.releaseMouse();
  camera = craft::core::Camera{};
  window.setFramebufferSize(craft::app::kBenchmarkFramebufferWidth,
                            craft::app::kBenchmarkFramebufferHeight);
  window.setVSync(false);
  benchmark.start();
}

void printBenchmarkResult(const craft::app::BenchmarkSnapshot& result,
                          int framebufferWidth, int framebufferHeight) {
  std::cout << std::fixed << std::setprecision(4)
            << "benchmark_result sample_count=" << result.measuredFrames
            << " average_ms=" << result.averageMilliseconds
            << " average_fps=" << result.averageFramesPerSecond
            << " resolution=" << framebufferWidth << 'x' << framebufferHeight
            << " workload=" << kStarterWorkload << '\n'
            << "benchmark_note=replace the tetrahedron with your voxel scene "
               "between the beginSceneFrame and endSceneFrame calls\n";
}

}  // namespace

int main() {
  try {
    craft::graphics::Window window(kInitialWidth, kInitialHeight,
                                   "Breachpoint");
    craft::graphics::Shader shader(
        craft::core::assetPath("shaders/basic.vert"),
        craft::core::assetPath("shaders/basic.frag"));
    const craft::graphics::Example3DMesh exampleMesh;
    craft::core::Camera camera;
    craft::app::CameraController controller(window, camera);
    craft::app::AppUi appUi(window);
    craft::app::SceneBenchmark benchmark;
    const std::vector<craft::core::Aabb> exampleBounds{
        {{-0.75F, -0.6F, -0.75F}, {0.75F, 0.8F, 0.55F}}};

    const bool commandLineBenchmark =
        craft::app::benchmarkRequestedFromEnvironment();
    if (commandLineBenchmark) {
      startBenchmark(window, benchmark, camera, controller);
    }

    // The start screen is skipped when benchmarking from the command line.
    std::optional<craft::app::TitleScreen> titleScreen;
    if (!commandLineBenchmark) {
      titleScreen.emplace(window);
    }

    glEnable(GL_DEPTH_TEST);
    double previousTime = glfwGetTime();

    while (!window.shouldClose()) {
      window.pollEvents();
      const double now = glfwGetTime();
      const float deltaSeconds = static_cast<float>(
          std::clamp(now - previousTime, 0.0, 0.1));
      previousTime = now;
      if (glfwGetWindowAttrib(window.nativeHandle(), GLFW_ICONIFIED)) {
        controller.releaseMouse();
        window.swapBuffers();
        continue;
      }

      auto [framebufferWidth, framebufferHeight] = window.framebufferSize();
      if (framebufferWidth == 0 || framebufferHeight == 0) {
        controller.releaseMouse();
        window.swapBuffers();
        continue;
      }

      if (titleScreen) {
        titleScreen->update();
        if (!titleScreen->finished()) {
          titleScreen->draw(framebufferWidth, framebufferHeight);
          window.swapBuffers();
          continue;
        }
        // Free the title screen's GPU resources once the game starts.
        titleScreen.reset();
      }

      const auto benchmarkState = benchmark.snapshot();
      controller.update(deltaSeconds, !benchmarkState.running,
                         appUi.wantsKeyboardInput());
      appUi.beginFrame(controller.mouseCaptured());
      const auto hit = craft::core::nearestBoxHit(camera.ray(), exampleBounds);
      appUi.setDiagnostics({camera.position(), camera.forward(),
                            camera.yawDegrees(), camera.pitchDegrees(),
                            controller.mouseCaptured(),
                            hit ? std::optional<float>(hit->t) : std::nullopt});
      if (appUi.draw(benchmarkState) && !benchmarkState.running) {
        startBenchmark(window, benchmark, camera, controller);
        const auto resizedFramebuffer = window.framebufferSize();
        framebufferWidth = resizedFramebuffer.first;
        framebufferHeight = resizedFramebuffer.second;
      }

      benchmark.beginSceneFrame();

      glViewport(0, 0, framebufferWidth, framebufferHeight);
      glClearColor(0.08F, 0.10F, 0.14F, 1.0F);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      shader.use();
      const float aspect = static_cast<float>(framebufferWidth) /
                           static_cast<float>(framebufferHeight);
      shader.setMat4("uMvp", camera.viewProjection(aspect));
      shader.setVec3("uTint", glm::vec3(1.0F));
      exampleMesh.draw();

      const bool benchmarkCompleted = benchmark.endSceneFrame();
      if (benchmarkCompleted) {
        printBenchmarkResult(benchmark.snapshot(), framebufferWidth,
                             framebufferHeight);
      }

      // Dear ImGui and presentation remain outside the measured scene region.
      appUi.render();
      window.swapBuffers();

      if (benchmarkCompleted) {
        window.setVSync(true);
        if (commandLineBenchmark) {
          window.requestClose();
        }
      }
    }
  } catch (const std::exception& error) {
    std::cerr << "Craft failed: " << error.what() << '\n';
    return 1;
  }

  return 0;
}
