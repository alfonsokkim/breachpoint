#include <algorithm>
#include <array>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/geometric.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "app/AppUi.h"
#include "app/CameraController.h"
#include "app/SceneBenchmark.h"
#include "app/TitleScreen.h"
#include "core/AssetPath.h"
#include "core/Camera.h"
#include "core/FireControl.h"
#include "core/MuzzleFlash.h"
#include "core/PlayerMovement.h"
#include "core/Shooting.h"
#include "core/ShotParticles.h"
#include "core/TrainingRound.h"
#include "graphics/Character.h"
#include "graphics/Chunk.h"
#include "graphics/FrameCapture.h"
#include "graphics/OpenGL.h"
#include "graphics/PointEffects.h"
#include "graphics/Shader.h"
#include "graphics/ScenePost.h"
#include "graphics/SpotShadows.h"
#include "graphics/Texture.h"
#include "graphics/Weapon.h"
#include "graphics/Window.h"

using namespace craft;

int main() {
  try {
    graphics::Window window(1280, 720, "Breachpoint");
    graphics::Shader shader(core::assetPath("shaders/chunk.vert"), core::assetPath("shaders/chunk.frag"));
    const graphics::Texture atlas(core::assetPath("textures/texture.png"), graphics::Texture::Magnification::Nearest);
    const graphics::Texture skin(core::assetPath("textures/steve.png"), graphics::Texture::Magnification::Nearest);
    graphics::Chunk chunk(atlas);
    const graphics::Weapon weapon;
    const graphics::PointEffects pointEffects;
    const auto ground = core::makeTrainingGround(0);
    const graphics::SpotShadows shadows(chunk, ground);
    graphics::ScenePost post;
    std::vector<core::Aabb> solids = chunk.getSolidBounds();
    for (const auto& light : ground.lights) {
      solids.push_back({{light.position.x - 0.07F, 1, light.position.z - 0.07F},
                        {light.position.x + 0.07F, 8, light.position.z + 0.07F}});
      solids.push_back({{light.position.x - 0.3F, 1, light.position.z - 0.3F},
                        {light.position.x + 0.3F, 1.12F, light.position.z + 0.3F}});
    }
    core::TrainingRound round;
    core::Camera camera(ground.spawn);
    app::CameraController controller(window, camera);
    app::AppUi appUi(window);
    app::SceneBenchmark benchmark;
    core::FireControl fireControl;
    core::MuzzleFlash muzzleFlash;
    core::ShotParticles particles;
    std::vector<std::unique_ptr<graphics::Character>> models;
    std::vector<core::Aabb> targetBoxes;
    std::vector<std::pair<std::size_t, std::size_t>> owners;
    std::vector<graphics::EffectPoint> effects;
    std::optional<core::ShotHit> lastShot;
    float feedback = 0;
    const auto reset = [&](core::Difficulty difficulty) {
      round.reset(ground, difficulty);
      models.clear();
      for (const auto& target : ground.targets)
        models.push_back(std::make_unique<graphics::Character>(skin, target.feet, !target.civilian));
      controller.releaseMouse();
      camera = core::Camera(ground.spawn);
      camera.look(ground.yaw + 90, 20);
      fireControl = {}; muzzleFlash.reset(); particles.clear(); effects.clear();
      lastShot.reset(); feedback = 0;
    };
    reset(core::Difficulty::Normal);
    const auto startBenchmark = [&] {
      reset(round.difficulty);
      appUi.settings() = {};
      window.setFramebufferSize(app::kBenchmarkFramebufferWidth, app::kBenchmarkFramebufferHeight);
      window.setVSync(false); benchmark.start();
    };
    const bool commandLineBenchmark = app::benchmarkRequestedFromEnvironment();
    const char* screenshotPath = std::getenv("CRAFT_SCREENSHOT");
    bool screenshotSaved = false;
    if (commandLineBenchmark) startBenchmark();
    std::optional<app::TitleScreen> title;
    if (!commandLineBenchmark) title.emplace(window);
    std::array<bool, GLFW_KEY_LAST + 1> previousKeys{};
    double previousTime = glfwGetTime();
    glEnable(GL_DEPTH_TEST);

    while (!window.shouldClose()) {
      window.pollEvents();
      const bool trigger = window.consumeLeftClick();
      std::array<bool, GLFW_KEY_LAST + 1> pressed{};
      for (int key : {GLFW_KEY_R, GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_N, GLFW_KEY_P, GLFW_KEY_B}) {
        const bool down = glfwGetKey(window.nativeHandle(), key) == GLFW_PRESS;
        pressed[key] = down && !previousKeys[key]; previousKeys[key] = down;
      }
      const double now = glfwGetTime();
      const float elapsed = static_cast<float>(std::max(now - previousTime, 0.0));
      const float dt = std::min(elapsed, 0.1F);
      previousTime = now;
      auto [width, height] = window.framebufferSize();
      if (!width || !height || glfwGetWindowAttrib(window.nativeHandle(), GLFW_ICONIFIED)) {
        controller.releaseMouse(); (void)fireControl.update(0, false, false);
        muzzleFlash.reset(); particles.clear(); window.swapBuffers(); continue;
      }
      if (title) {
        title->update();
        if (!title->finished()) { title->draw(width, height); window.swapBuffers(); continue; }
        title.reset();
      }
      bool measuring = benchmark.snapshot().running;
      const bool focused = glfwGetWindowAttrib(window.nativeHandle(), GLFW_FOCUSED) == GLFW_TRUE;
      if (!measuring && focused && !appUi.wantsKeyboardInput()) {
        if (pressed[GLFW_KEY_R]) reset(round.difficulty);
        if (pressed[GLFW_KEY_1]) reset(core::Difficulty::Easy);
        if (pressed[GLFW_KEY_2]) reset(core::Difficulty::Normal);
        if (pressed[GLFW_KEY_3]) reset(core::Difficulty::Hard);
        if (pressed[GLFW_KEY_N]) appUi.settings().nightVision = !appUi.settings().nightVision;
        if (pressed[GLFW_KEY_P]) appUi.settings().brdf = !appUi.settings().brdf;
        if (pressed[GLFW_KEY_B]) { startBenchmark(); measuring = true; }
      }
      const bool ended = round.state != core::RoundState::Ready && round.state != core::RoundState::Playing;
      const auto oldPosition = camera.position();
      controller.update(dt, !measuring && !ended, appUi.wantsKeyboardInput());
      camera.setPosition(core::movePlayer(oldPosition, camera.position(), solids));
      const bool active = !measuring && controller.mouseCaptured() && !appUi.wantsKeyboardInput();
      if (active) round.start();
      if (active || (!measuring && round.state != core::RoundState::Playing))
        round.update(elapsed, camera.position(), solids);
      targetBoxes.clear(); owners.clear();
      for (std::size_t i = 0; i < models.size(); ++i) {
        if (round.targets[i].health <= 0) continue;
        const auto& boxes = models[i]->getHitBounds();
        for (std::size_t part = 0; part < boxes.size(); ++part) {
          targetBoxes.push_back(boxes[part]); owners.emplace_back(i, part);
        }
      }
      feedback = std::max(0.0F, feedback - elapsed); muzzleFlash.update(elapsed);
      if (!measuring) particles.update(elapsed, solids);
      if (fireControl.update(elapsed, trigger, active && round.state == core::RoundState::Playing)) {
        lastShot = core::resolveShot(camera.ray(), solids, targetBoxes);
        ++round.shots; feedback = 0.15F; muzzleFlash.trigger();
        particles.emitMuzzle(core::muzzlePosition(camera.ray(), solids), camera.forward());
        if (lastShot) {
          particles.emitImpact(*lastShot);
          if (lastShot->kind == core::ShotHitKind::Target) {
            const auto [target, part] = owners[lastShot->index]; round.hit(target, part == 0 || part == 6);
          }
        }
      }
      if (round.state != core::RoundState::Ready && round.state != core::RoundState::Playing)
        controller.releaseMouse();
      for (std::size_t i = 0; i < models.size(); ++i)
        models[i]->setPose(round.targets[i].spawn.feet, 0, round.targets[i].fall);

      effects.clear();
      const auto muzzle = core::muzzlePosition(camera.ray(), solids);
      glm::vec3 lightPosition = muzzle;
      float lightIntensity = measuring ? 0 : muzzleFlash.intensity();
      if (!measuring) {
        for (const auto& p : particles.live()) {
          auto colour = p.colour; colour.a *= p.remaining / p.lifetime;
          effects.push_back({p.position, colour, p.size, 0});
        }
        if (muzzleFlash.intensity() > 0)
          effects.push_back({muzzle, {1, 0.85F, 0.45F, muzzleFlash.intensity()}, 0.22F, 1});
        for (const auto& target : round.targets) {
          if (target.flash <= 0 || target.health <= 0) continue;
          const glm::vec3 origin = target.spawn.feet + glm::vec3(0, 1.4F, 0);
          const auto enemyMuzzle = core::muzzlePosition({origin, glm::normalize(camera.position() - origin)}, solids);
          effects.push_back({enemyMuzzle, {1, 0.7F, 0.25F, target.flash / 0.12F}, 0.3F, 1});
          if (lightIntensity == 0) { lightPosition = enemyMuzzle; lightIntensity = target.flash / 0.12F; }
        }
      }
      app::CameraDiagnostics diagnostics;
      diagnostics.position = camera.position(); diagnostics.forward = camera.forward();
      diagnostics.yaw = camera.yawDegrees(); diagnostics.pitch = camera.pitchDegrees();
      diagnostics.mouseCaptured = controller.mouseCaptured(); diagnostics.worldFaces = chunk.getFaceCount();
      diagnostics.lastShotHit = lastShot; diagnostics.feedbackSeconds = feedback;
      diagnostics.effectPoints = static_cast<int>(effects.size()); diagnostics.round = &round; diagnostics.levelName = ground.name;
      int parts = 0;
      for (const auto& model : models) parts += static_cast<int>(model->partCount());
      diagnostics.sceneTriangles = chunk.getFaceCount() * 2 + parts * 12 + 24 + round.hostilesRemaining() * 24 + 16 * 12 + 1;
      diagnostics.sceneDraws = 4 + parts + round.hostilesRemaining() * 2 + 16 + (!effects.empty() ? 1 : 0);
      appUi.beginFrame(controller.mouseCaptured()); appUi.setDiagnostics(diagnostics);
      if (appUi.draw(benchmark.snapshot()) && !measuring) {
        startBenchmark(); effects.clear(); lightIntensity = 0;
        const auto size = window.framebufferSize(); width = size.first; height = size.second;
      }
      benchmark.beginSceneFrame();
      post.begin(width, height); glEnable(GL_CULL_FACE);
      const float aspect = static_cast<float>(width) / height;
      shader.use(); shader.setMat4("uViewProjection", camera.viewProjection(aspect));
      shader.setMat4("uModel", chunk.getModel()); shader.setInt("uTex", 0);
      shader.setInt("uTextured", 0); shader.setInt("uVertexColour", 1);
      shader.setVec3("uTint", {1, 1, 1}); shader.setVec3("uEyePosition", camera.position());
      shader.setVec3("uMuzzlePosition", lightPosition); shader.setFloat("uMuzzleIntensity", lightIntensity);
      shader.setFloat("uSpecular", 0.08F); shader.setFloat("uShininess", 16);
      shader.setInt("uPbr", appUi.settings().brdf);
      shader.setFloat("uRoughness", 0.7F); shader.setFloat("uMetallic", 0);
      shader.setFloat("uEmissive", 0);
      shadows.bind(shader);
      for (int i = 0; i < 4; ++i) {
        shader.setVec3("uSpotPositions[" + std::to_string(i) + "]", ground.lights[i].position);
        shader.setVec3("uSpotDirections[" + std::to_string(i) + "]", ground.lights[i].aim - ground.lights[i].position);
      }
      chunk.draw(); shader.setInt("uVertexColour", 0);
      shader.setFloat("uMetallic", 0.7F); shader.setFloat("uRoughness", 0.4F);
      for (const auto& light : ground.lights) {
        const auto p = light.position;
        const auto box = [&](glm::vec3 minimum, glm::vec3 size) {
          weapon.drawBox(shader, glm::scale(glm::translate(glm::mat4(1), minimum), size), {0.09F, 0.11F, 0.12F});
        };
        box({p.x - 0.3F, 1, p.z - 0.3F}, {0.6F, 0.12F, 0.6F});
        box({p.x - 0.07F, 1, p.z - 0.07F}, {0.14F, p.y - 1, 0.14F});
        const auto frame = core::weaponFrame({p, light.aim - p});
        weapon.drawBox(shader, glm::scale(glm::translate(frame, {-0.7F, -0.18F, -0.25F}), {1.4F, 0.36F, 0.5F}), {0.07F, 0.08F, 0.09F});
        shader.setFloat("uEmissive", 6);
        weapon.drawBox(shader, glm::scale(glm::translate(frame, {-0.59F, -0.125F, -0.275F}), {1.18F, 0.25F, 0.02F}), {1, 0.85F, 0.65F});
        shader.setFloat("uEmissive", 0);
      }
      for (std::size_t i = 0; i < models.size(); ++i) {
        const auto& target = round.targets[i];
        shader.setInt("uTextured", 0);
        shader.setFloat("uMetallic", 0); shader.setFloat("uRoughness", 0.8F);
        shader.setVec3("uTint", target.spawn.civilian ? glm::vec3(0.12F, 0.55F, 0.85F) : glm::vec3(0.8F, 0.19F, 0.12F));
        shader.setFloat("uSpecular", 0.08F); shader.setFloat("uShininess", 16);
        models[i]->draw(shader);
        if (!target.spawn.civilian && target.health > 0) {
          const glm::vec3 origin = target.spawn.feet + glm::vec3(0, 1.4F, 0);
          shader.setFloat("uRoughness", appUi.settings().roughness);
          shader.setFloat("uMetallic", appUi.settings().metallic);
          weapon.draw(shader, core::weaponFrame({origin, glm::normalize(camera.position() - origin)}));
        }
      }
      shader.setFloat("uRoughness", appUi.settings().roughness);
      shader.setFloat("uMetallic", appUi.settings().metallic);
      weapon.draw(shader, core::weaponFrame(camera.ray()));
      pointEffects.draw(effects, camera.viewProjection(aspect), height);
      post.present(appUi.settings().nightVision, measuring ? 0 : static_cast<float>(now));
      const bool completed = benchmark.endSceneFrame();
      if (completed) std::cout << std::fixed << std::setprecision(4)
          << "benchmark_result sample_count=" << benchmark.snapshot().measuredFrames
          << " average_ms=" << benchmark.snapshot().averageMilliseconds
          << " resolution=" << width << 'x' << height << " workload=training-courtyard\n";
      appUi.render();
      if (screenshotPath && !screenshotSaved && (!commandLineBenchmark || completed)) {
        screenshotSaved = graphics::saveFrame(screenshotPath, width, height);
        if (!screenshotSaved) { std::cerr << "Could not save screenshot\n"; screenshotPath = nullptr; }
      }
      window.swapBuffers();
      if (completed) { window.setVSync(true); if (commandLineBenchmark) window.requestClose(); }
    }
  } catch (const std::exception& error) {
    std::cerr << "Breachpoint failed: " << error.what() << '\n'; return 1;
  }
}
