#include "app/CustomUi.h"

#include <cmath>
#include "imgui.h"

namespace craft::app {

void CustomUi::drawContents(bool frozen) {
  if (diagnostics_.round) {
    const auto& r = *diagnostics_.round;
    ImGui::TextUnformatted(diagnostics_.levelName);
    ImGui::Separator();
    ImGui::Text("%s%s", core::roundStateName(r.state),
        r.state == core::RoundState::Playing && !diagnostics_.mouseCaptured ? " / PAUSED" : "");
    ImGui::Text("Health %d / 100", r.health);
    ImGui::ProgressBar(r.health / 100.0F, {-1, 8}, "");
    const int time = static_cast<int>(std::ceil(r.timeRemaining));
    ImGui::Text("Time %02d:%02d", time / 60, time % 60);
    ImGui::Text("Hostiles %d   Civilians %d / 2", r.hostilesRemaining(), r.civilianStrikes);
    ImGui::Text("Shots %d", r.shots);
    ImGui::Text("Difficulty: %s", r.difficulty == core::Difficulty::Easy ? "Easy" :
        r.difficulty == core::Difficulty::Hard ? "Hard" : "Normal");
    ImGui::SeparatorText("Orders");
    ImGui::TextWrapped("Eliminate the armed targets. Spare the blue civilians. One headshot or two body hits neutralises a hostile.");
    ImGui::TextWrapped("Hostiles return fire after sustained sight. Break sight behind a pillar to reset their exposure timer.");
    ImGui::TextUnformatted("Tab: begin / pause / release mouse");
    ImGui::TextUnformatted("WASD + mouse: move / aim");
    ImGui::TextUnformatted("Left click: fire   R: restart");
    ImGui::TextUnformatted("1 / 2 / 3: easy / normal / hard");
    ImGui::TextUnformatted("N: night vision   P: BRDF");
    ImGui::TextUnformatted("B: run fixed benchmark");
  }
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
        diagnostics_.lastShotHit->kind == core::ShotHitKind::Solid ? "wall" : "target",
        diagnostics_.lastShotHit->distance);
  }
  const auto size = ImGui::GetIO().DisplaySize;
  const ImVec2 c(size.x * 0.5F, size.y * 0.5F);
  auto* draw = ImGui::GetForegroundDrawList();
  const ImU32 colour = diagnostics_.feedbackSeconds > 0 ? IM_COL32(240, 195, 100, 255) : IM_COL32_WHITE;
  draw->AddLine({c.x - 5, c.y}, {c.x + 5, c.y}, colour);
  draw->AddLine({c.x, c.y - 5}, {c.x, c.y + 5}, colour);
  if (diagnostics_.round && diagnostics_.round->state != core::RoundState::Playing) {
    const auto state = diagnostics_.round->state;
    const char* title = core::roundStateName(state);
    const char* prompt = state == core::RoundState::Ready ? "TAB TO BEGIN" : "R TO RESTART";
    draw->AddRectFilled({c.x - 150, c.y - 72}, {c.x + 150, c.y - 12}, IM_COL32(8, 11, 15, 230));
    draw->AddText({c.x - ImGui::CalcTextSize(title).x / 2, c.y - 62}, IM_COL32_WHITE, title);
    draw->AddText({c.x - ImGui::CalcTextSize(prompt).x / 2, c.y - 38}, IM_COL32(190, 200, 210, 255), prompt);
  }
}

void CustomUi::setDiagnostics(const CameraDiagnostics& diagnostics) { diagnostics_ = diagnostics; }

}
