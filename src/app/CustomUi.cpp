#include "app/CustomUi.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
#include "imgui.h"

namespace craft::app {
namespace {
constexpr ImU32 kWhite = IM_COL32(235, 238, 235, 255);
constexpr ImU32 kMuted = IM_COL32(160, 173, 179, 255);
constexpr ImU32 kGold = IM_COL32(223, 190, 112, 255);
constexpr ImU32 kRed = IM_COL32(244, 100, 87, 255);

void text(ImDrawList* draw, ImVec2 p, float size, ImU32 colour, const char* value) {
  draw->AddText(ImGui::GetFont(), size, p, colour, value);
}

void centred(ImDrawList* draw, float x, float y, float size, ImU32 colour, const char* value) {
  const float width = ImGui::GetFont()->CalcTextSizeA(size, std::numeric_limits<float>::max(), 0, value).x;
  text(draw, {x - width * 0.5F, y}, size, colour, value);
}

void panel(ImDrawList* draw, ImVec2 p, ImVec2 size) {
  draw->AddRectFilled(p, {p.x + size.x, p.y + size.y}, IM_COL32(8, 13, 18, 190));
  draw->AddRect(p, {p.x + size.x, p.y + size.y}, IM_COL32(200, 215, 220, 45));
}
}

void CustomUi::drawHud(const BenchmarkSnapshot& benchmark) {
  if (!diagnostics_.round) return;
  const auto& r = *diagnostics_.round;
  const auto screen = ImGui::GetIO().DisplaySize;
  const float s = std::clamp(std::min(screen.x / 1100.0F, screen.y / 650.0F), 0.65F, 1.0F);
  const float margin = 24 * s;
  const float cx = screen.x * 0.5F;
  const float cy = screen.y * 0.5F;
  const double now = ImGui::GetTime();
  auto* draw = ImGui::GetBackgroundDrawList();
  if (r.state == core::RoundState::Ready) {
    damageUntil_ = 0; civilianUntil_ = 0;
  } else {
    if (r.health < previousHealth_) damageUntil_ = now + 0.35;
    if (r.civilianStrikes > previousStrikes_) civilianUntil_ = now + 2.0;
  }
  previousHealth_ = r.health; previousStrikes_ = r.civilianStrikes;
  if (now < damageUntil_) {
    const int alpha = static_cast<int>(130 * (damageUntil_ - now) / 0.35);
    const ImU32 red = IM_COL32(170, 30, 20, alpha);
    const ImU32 clear = IM_COL32(170, 30, 20, 0);
    const float edge = 45 * s;
    draw->AddRectFilledMultiColor({0, 0}, {screen.x, edge}, red, red, clear, clear);
    draw->AddRectFilledMultiColor({0, screen.y - edge}, screen, clear, clear, red, red);
    draw->AddRectFilledMultiColor({0, 0}, {edge, screen.y}, red, clear, clear, red);
    draw->AddRectFilledMultiColor({screen.x - edge, 0}, screen, clear, red, red, clear);
  }

  draw->AddRectFilled({margin, margin}, {margin + 3 * s, margin + 40 * s}, kGold);
  text(draw, {margin + 13 * s, margin}, 18 * s, kWhite, "BREACHPOINT");
  text(draw, {margin + 13 * s, margin + 25 * s}, 11 * s, kMuted, "COURTYARD / LIVE FIRE");
  const char* difficulty = r.difficulty == core::Difficulty::Easy ? "EASY" :
      r.difficulty == core::Difficulty::Hard ? "HARD" : "NORMAL";
  text(draw, {margin + 13 * s, margin + 43 * s}, 10 * s, kGold, difficulty);

  panel(draw, {cx - 68 * s, margin}, {136 * s, 72 * s});
  const int seconds = std::max(0, static_cast<int>(std::ceil(r.timeRemaining)));
  char value[96];
  centred(draw, cx, margin + 9 * s, 10 * s, kMuted, "TIME REMAINING");
  std::snprintf(value, sizeof(value), "%02d:%02d", seconds / 60, seconds % 60);
  centred(draw, cx, margin + 25 * s, 28 * s, seconds <= 15 ? kRed : kWhite, value);
  const char* status = benchmark.running ? "BENCHMARK" :
      r.state == core::RoundState::Playing ? (diagnostics_.mouseCaptured ? "LIVE" : "PAUSED") :
      core::roundStateName(r.state);
  centred(draw, cx, margin + 58 * s, 9 * s, kGold, status);

  const float right = screen.x - margin - 208 * s;
  panel(draw, {right, margin}, {208 * s, 80 * s});
  text(draw, {right + 12 * s, margin + 10 * s}, 10 * s, kMuted, "HOSTILES REMAINING");
  const int total = static_cast<int>(std::count_if(r.targets.begin(), r.targets.end(),
      [](const core::TargetState& t) { return !t.spawn.civilian; }));
  std::snprintf(value, sizeof(value), "%02d / %02d", r.hostilesRemaining(), total);
  text(draw, {right + 12 * s, margin + 27 * s}, 24 * s, kWhite, value);
  std::snprintf(value, sizeof(value), "CIVILIAN STRIKES  %d / 2", r.civilianStrikes);
  text(draw, {right + 12 * s, margin + 61 * s}, 10 * s,
       r.civilianStrikes ? kRed : kMuted, value);

  const float bottom = screen.y - margin;
  panel(draw, {margin, bottom - 76 * s}, {184 * s, 76 * s});
  text(draw, {margin + 12 * s, bottom - 65 * s}, 10 * s, kMuted, "OPERATOR HEALTH");
  std::snprintf(value, sizeof(value), "%03d", r.health);
  const ImU32 healthColour = r.health <= 30 ? kRed : kWhite;
  text(draw, {margin + 12 * s, bottom - 48 * s}, 29 * s, healthColour, value);
  text(draw, {margin + 74 * s, bottom - 34 * s}, 11 * s, kMuted, "/ 100");
  for (int i = 0; i < 10; ++i) {
    const float x = margin + (12 + i * 16) * s;
    draw->AddRectFilled({x, bottom - 13 * s}, {x + 13 * s, bottom - 9 * s},
        r.health > i * 10 ? healthColour : IM_COL32(80, 94, 100, 100));
  }
  const float weaponX = screen.x - margin - 174 * s;
  panel(draw, {weaponX, bottom - 64 * s}, {174 * s, 64 * s});
  text(draw, {weaponX + 12 * s, bottom - 53 * s}, 11 * s, kGold, "SEMI / 50 M");
  std::snprintf(value, sizeof(value), "SHOTS FIRED  %03d", r.shots);
  text(draw, {weaponX + 12 * s, bottom - 34 * s}, 11 * s, kWhite, value);
  text(draw, {weaponX + 12 * s, bottom - 16 * s}, 9 * s,
       settings_.nightVision ? IM_COL32(135, 225, 165, 255) : kMuted,
       settings_.nightVision ? "NVG ON  [N]" : "NVG OFF  [N]");
  centred(draw, cx, bottom - 13 * s, 10 * s, kMuted, "TAB PAUSE   R RESTART   H CONTROLS");

  const bool playing = r.state == core::RoundState::Playing && diagnostics_.mouseCaptured;
  if (!benchmark.running && (playing || r.state == core::RoundState::Ready)) {
    const ImU32 colour = diagnostics_.feedbackSeconds > 0 && diagnostics_.lastShotHit &&
        diagnostics_.lastShotHit->kind == core::ShotHitKind::Solid ? kGold : kWhite;
    for (float direction : {-1.0F, 1.0F}) {
      draw->AddLine({cx + direction * 4 * s, cy}, {cx + direction * 10 * s, cy}, IM_COL32_BLACK, 3 * s);
      draw->AddLine({cx, cy + direction * 4 * s}, {cx, cy + direction * 10 * s}, IM_COL32_BLACK, 3 * s);
      draw->AddLine({cx + direction * 4 * s, cy}, {cx + direction * 10 * s, cy}, colour, s);
      draw->AddLine({cx, cy + direction * 4 * s}, {cx, cy + direction * 10 * s}, colour, s);
      if (diagnostics_.feedbackSeconds > 0 && diagnostics_.lastShotHit &&
          diagnostics_.lastShotHit->kind == core::ShotHitKind::Target) {
        const ImU32 hit = diagnostics_.lastShotCivilian ? kRed : IM_COL32(150, 230, 160, 255);
        draw->AddLine({cx + direction * 8 * s, cy + 8 * s}, {cx + direction * 13 * s, cy + 13 * s}, hit, 2 * s);
        draw->AddLine({cx + direction * 8 * s, cy - 8 * s}, {cx + direction * 13 * s, cy - 13 * s}, hit, 2 * s);
      }
    }
  }
  if (playing) {
    float exposure = 0;
    for (const auto& target : r.targets)
      if (!target.spawn.civilian && target.health > 0) exposure = std::max(exposure, target.exposure);
    if (now < civilianUntil_) {
      centred(draw, cx, cy + 44 * s, 13 * s, kRed, "CIVILIAN HIT / WATCH YOUR FIRE");
    } else if (exposure > 0) {
      const float ratio = std::min(1.0F, exposure / r.exposureDelay());
      centred(draw, cx, screen.y - 112 * s, 11 * s, ratio >= 1 ? kRed : kGold,
               ratio >= 1 ? "UNDER FIRE / FIND COVER" : "EXPOSED / BREAK LINE OF SIGHT");
      draw->AddRectFilled({cx - 70 * s, screen.y - 91 * s}, {cx + 70 * s, screen.y - 88 * s}, IM_COL32(25, 32, 38, 220));
      draw->AddRectFilled({cx - 70 * s, screen.y - 91 * s}, {cx + (ratio * 140 - 70) * s, screen.y - 88 * s}, ratio >= 1 ? kRed : kGold);
    }
  }
  if (benchmark.running) {
    std::snprintf(value, sizeof(value), "BENCHMARK  %zu / %zu", benchmark.measuredFrames, benchmark.requiredMeasuredFrames);
    centred(draw, cx, margin + 90 * s, 11 * s, kGold, value);
  } else if (!playing) {
    const bool ready = r.state == core::RoundState::Ready;
    const bool paused = r.state == core::RoundState::Playing;
    const bool won = r.state == core::RoundState::Won;
    const float y = cy - 122 * s;
    panel(draw, {cx - 200 * s, y}, {400 * s, 102 * s});
    centred(draw, cx, y + 15 * s, 23 * s, ready || paused ? kWhite : won ? kGold : kRed,
             ready ? "COURTYARD EXERCISE" : paused ? "PAUSED" : core::roundStateName(r.state));
    centred(draw, cx, y + 49 * s, 11 * s, kMuted,
             ready ? "Eliminate armed targets. Protect civilians." : paused ?
             "The round is paused while the mouse is released." : won ?
             "All hostiles neutralised." : r.state == core::RoundState::Dead ?
             "You were exposed to enemy fire." : r.state == core::RoundState::Timeout ?
             "The time limit expired." : "Two civilians were hit.");
    centred(draw, cx, y + 77 * s, 11 * s, kGold,
             ready ? "TAB BEGIN   1 / 2 / 3 DIFFICULTY" : paused ? "TAB RESUME   H CONTROLS" : "R RESTART   1 / 2 / 3 DIFFICULTY");
  }
}

void CustomUi::drawContents(bool frozen) {
  ImGui::TextWrapped("Tab releases the mouse and pauses the round.");
  ImGui::TextUnformatted("WASD + mouse: move / aim");
  ImGui::TextUnformatted("Left click: fire   R: restart");
  ImGui::TextUnformatted("1 / 2 / 3: easy / normal / hard");
  ImGui::TextUnformatted("N: night vision   P: BRDF");
  ImGui::TextUnformatted("B: benchmark   H / F3: controls");
  ImGui::BeginDisabled(frozen);
  ImGui::SeparatorText("Rendering");
  ImGui::Checkbox("Night vision", &settings_.nightVision);
  ImGui::Checkbox("Cook-Torrance BRDF", &settings_.brdf);
  ImGui::SetNextItemWidth(135);
  ImGui::SliderFloat("Roughness", &settings_.roughness, 0.08F, 1);
  ImGui::SetNextItemWidth(135);
  ImGui::SliderFloat("Metallic", &settings_.metallic, 0, 1);
  ImGui::EndDisabled();
  if (ImGui::CollapsingHeader("Diagnostics")) {
    const auto p = diagnostics_.position;
    ImGui::Text("Position %.2f %.2f %.2f", p.x, p.y, p.z);
    ImGui::Text("Faces %d", diagnostics_.worldFaces);
    ImGui::Text("Triangles %d   Draws %d", diagnostics_.sceneTriangles, diagnostics_.sceneDraws);
    ImGui::Text("Effect points %d", diagnostics_.effectPoints);
    if (diagnostics_.lastShotHit) ImGui::Text("Last hit: %s / %.2f m",
        diagnostics_.lastShotHit->kind == core::ShotHitKind::Solid ? "wall" : "target", diagnostics_.lastShotHit->distance);
  }
}

void CustomUi::setDiagnostics(const CameraDiagnostics& diagnostics) { diagnostics_ = diagnostics; }

}
