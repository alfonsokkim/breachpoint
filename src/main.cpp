#include <iomanip>
#include <iostream>
#include <optional>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>

#include "app/AppUi.h"
#include "app/SceneBenchmark.h"
#include "app/TitleScreen.h"
#include "core/AssetPath.h"
#include "graphics/Chunk.h"
#include "graphics/OpenGL.h"
#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/Window.h"

namespace {

constexpr int kInitialWidth = 1280;
constexpr int kInitialHeight = 720;
constexpr const char* kStarterWorkload = "starter-3d-tetrahedron";

void startBenchmark(craft::graphics::Window& window,
                    craft::app::SceneBenchmark& benchmark) {
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

glm::mat4 projection(int framebufferWidth, int framebufferHeight) {
  const float aspect = static_cast<float>(framebufferWidth) /
                       static_cast<float>(framebufferHeight);
  return glm::perspective(glm::radians(60.0F), aspect, 0.1F, 100.0F);
}

// Looks at the demo hut's doorway from just outside the platform.
glm::mat4 fixedView() {
  return glm::lookAt(glm::vec3(7.0F, 10.0F, 25.0F),
                     glm::vec3(15.5F, 5.0F, 15.5F),
                     glm::vec3(0.0F, 1.0F, 0.0F));
}

}  // namespace

int main() {
  try {
    craft::graphics::Window window(kInitialWidth, kInitialHeight,
                                   "Breachpoint");
    craft::graphics::Shader chunkShader(
        craft::core::assetPath("shaders/chunk.vert"),
        craft::core::assetPath("shaders/chunk.frag"));
    const craft::graphics::Texture blockAtlas(
        craft::core::assetPath("textures/texture.png"),
        craft::graphics::Texture::Magnification::Nearest);
    const craft::graphics::Chunk chunk(blockAtlas);
    craft::app::AppUi appUi(window);
    craft::app::SceneBenchmark benchmark;

    const bool commandLineBenchmark =
        craft::app::benchmarkRequestedFromEnvironment();
    if (commandLineBenchmark) {
      startBenchmark(window, benchmark);
    }

    // The start screen is skipped when benchmarking from the command line.
    std::optional<craft::app::TitleScreen> titleScreen;
    if (!commandLineBenchmark) {
      titleScreen.emplace(window);
    }

    glEnable(GL_DEPTH_TEST);

    while (!window.shouldClose()) {
      window.pollEvents();

      auto [framebufferWidth, framebufferHeight] = window.framebufferSize();
      if (framebufferWidth == 0 || framebufferHeight == 0) {
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

      appUi.beginFrame();
      const auto benchmarkState = benchmark.snapshot();
      if (appUi.draw(benchmarkState) && !benchmarkState.running) {
        startBenchmark(window, benchmark);
        const auto resizedFramebuffer = window.framebufferSize();
        framebufferWidth = resizedFramebuffer.first;
        framebufferHeight = resizedFramebuffer.second;
      }

      benchmark.beginSceneFrame();

      glViewport(0, 0, framebufferWidth, framebufferHeight);
      glClearColor(0.08F, 0.10F, 0.14F, 1.0F);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // Chunk faces wind counter-clockwise, so hidden back faces are skipped.
      glEnable(GL_CULL_FACE);
      chunkShader.use();
      chunkShader.setMat4("uProj",
                          projection(framebufferWidth, framebufferHeight));
      chunkShader.setMat4("uView", fixedView());
      chunkShader.setMat4("uModel", chunk.getModel());
      chunkShader.setInt("uTex", 0);
      chunk.draw();

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
