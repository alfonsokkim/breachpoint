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
#include "core/FireControl.h"
#include "core/PlayerMovement.h"
#include "core/Shooting.h"
#include "core/SpatialQueries.h"
#include "graphics/Character.h"
#include "graphics/Chunk.h"
#include "graphics/OpenGL.h"
#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/Window.h"

namespace {

constexpr int kInitialWidth = 1280;
constexpr int kInitialHeight = 720;
constexpr const char* kWorldWorkload = "voxel-hut-and-character";

craft::core::Camera spawnCamera(bool targetInsideHut = false) {
  craft::core::Camera camera({11.5F, 6.6F, 19.5F});
  camera.look(targetInsideHut ? 45.0F : 0.0F,
               targetInsideHut ? 15.0F : 20.0F);
  return camera;
}

void startBenchmark(craft::graphics::Window& window,
                    craft::app::SceneBenchmark& benchmark,
                    craft::core::Camera& camera,
                    craft::app::CameraController& controller) {
  controller.releaseMouse();
  camera = spawnCamera();
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
            << " workload=" << kWorldWorkload << '\n'
            << "benchmark_note=demo hut and character; "
               "no optimisation comparison yet\n";
}

}  // namespace

int main() {
  try {
    craft::graphics::Window window(kInitialWidth, kInitialHeight,
                                   "Breachpoint");
    craft::graphics::Shader shader(
        craft::core::assetPath("shaders/chunk.vert"),
        craft::core::assetPath("shaders/chunk.frag"));
    const craft::graphics::Texture atlas(
        craft::core::assetPath("textures/texture.png"),
        craft::graphics::Texture::Magnification::Nearest);
    const craft::graphics::Chunk chunk(atlas);
    const craft::graphics::Texture steveSkin(
        craft::core::assetPath("textures/steve.png"),
        craft::graphics::Texture::Magnification::Nearest);
    craft::graphics::Character steve(steveSkin, {11.5F, 5.0F, 15.5F});
    craft::core::Camera camera = spawnCamera();
    craft::app::CameraController controller(window, camera);
    craft::app::AppUi appUi(window);
    craft::app::SceneBenchmark benchmark;
    craft::core::FireControl fireControl;
    std::optional<craft::core::ShotHit> lastShotHit;
    unsigned int shotsFired = 0;
    float feedbackSeconds = 0.0F;
    bool targetInsideHut = false;
    bool lastShotObstructed = false;
    bool demoKeyWasDown = false;

    const auto resetShootingDemo = [&] {
      camera = spawnCamera(targetInsideHut);
      steve.setFeetPosition({targetInsideHut ? 15.5F : 11.5F, 5.0F, 15.5F});
      fireControl = craft::core::FireControl{};
      lastShotHit.reset();
      shotsFired = 0;
      feedbackSeconds = 0.0F;
      lastShotObstructed = false;
    };

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
      const bool triggerPressed = window.consumeLeftClick();
      const bool demoKeyDown =
          glfwGetKey(window.nativeHandle(), GLFW_KEY_T) == GLFW_PRESS;
      const bool switchDemo = demoKeyDown && !demoKeyWasDown;
      demoKeyWasDown = demoKeyDown;
      const double now = glfwGetTime();
      const float deltaSeconds = static_cast<float>(
          std::clamp(now - previousTime, 0.0, 0.1));
      previousTime = now;
      if (glfwGetWindowAttrib(window.nativeHandle(), GLFW_ICONIFIED)) {
        controller.releaseMouse();
        (void)fireControl.update(0.0F, false, false);
        window.swapBuffers();
        continue;
      }

      auto [framebufferWidth, framebufferHeight] = window.framebufferSize();
      if (framebufferWidth == 0 || framebufferHeight == 0) {
        controller.releaseMouse();
        (void)fireControl.update(0.0F, false, false);
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
      const glm::vec3 previousPosition = camera.position();
      controller.update(deltaSeconds, !benchmarkState.running,
                         appUi.wantsKeyboardInput());
      camera.setPosition(craft::core::movePlayer(
          previousPosition, camera.position(), chunk.getSolidBounds()));
      const bool shootingEnabled = controller.mouseCaptured() &&
          !benchmarkState.running && !appUi.wantsKeyboardInput();
      if (switchDemo && shootingEnabled) {
        targetInsideHut = !targetInsideHut;
        resetShootingDemo();
      }
      feedbackSeconds = std::max(0.0F, feedbackSeconds - deltaSeconds);
      if (fireControl.update(deltaSeconds, triggerPressed, shootingEnabled)) {
        lastShotHit = craft::core::resolveShot(
            camera.ray(), chunk.getSolidBounds(), steve.getHitBounds());
        const auto targetHit = craft::core::nearestBoxHit(
            camera.ray(), steve.getHitBounds(), 50.0F);
        lastShotObstructed = lastShotHit && targetHit &&
            lastShotHit->kind == craft::core::ShotHitKind::Solid &&
            lastShotHit->distance <= targetHit->t;
        ++shotsFired;
        feedbackSeconds = 0.15F;
      }
      appUi.beginFrame(controller.mouseCaptured());
      const auto hit = craft::core::nearestBoxHit(camera.ray(), chunk.getSolidBounds());
      appUi.setDiagnostics({camera.position(), camera.forward(),
                            camera.yawDegrees(), camera.pitchDegrees(),
                            controller.mouseCaptured(),
                            hit ? std::optional<float>(hit->t) : std::nullopt,
                            chunk.getFaceCount(), lastShotHit,
                            shotsFired, feedbackSeconds,
                            targetInsideHut, lastShotObstructed});
      if (appUi.draw(benchmarkState) && !benchmarkState.running) {
        targetInsideHut = false;
        resetShootingDemo();
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
      glEnable(GL_CULL_FACE);
      const float aspect = static_cast<float>(framebufferWidth) /
                           static_cast<float>(framebufferHeight);
      shader.setMat4("uViewProjection", camera.viewProjection(aspect));
      shader.setMat4("uModel", chunk.getModel());
      shader.setInt("uTex", 0);
      chunk.draw();
      steve.draw(shader);

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
