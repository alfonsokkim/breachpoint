#include "app/CustomUi.h"

#include "imgui.h"

namespace craft::app {

void CustomUi::drawContents(bool interactionDisabled) {
  ImGui::SeparatorText("Camera");
  ImGui::TextUnformatted("Tab: capture / release mouse");
  ImGui::TextUnformatted("WASD: move   Mouse: look");
  ImGui::TextUnformatted(interactionDisabled ? "Frozen for benchmark" :
      (diagnostics_.mouseCaptured ? "Mouse captured" : "Mouse released"));
  const auto& p = diagnostics_.position;
  const auto& f = diagnostics_.forward;
  ImGui::Text("Position: %.2f %.2f %.2f", p.x, p.y, p.z);
  ImGui::Text("Yaw / pitch: %.1f / %.1f", diagnostics_.yaw, diagnostics_.pitch);
  ImGui::Text("Forward: %.2f %.2f %.2f", f.x, f.y, f.z);
  ImGui::SeparatorText("World");
  ImGui::TextUnformatted("Wall collision on");
  ImGui::TextWrapped("Walk through the doorway. Platform edges stop movement.");
  if (diagnostics_.solidHitDistance) {
    ImGui::Text("Block: %.2f units away", *diagnostics_.solidHitDistance);
  } else {
    ImGui::TextUnformatted("Block: none in aim");
  }
  ImGui::Text("Chunk faces: %d", diagnostics_.worldFaces);
  ImGui::Text("Scene triangles: %d", diagnostics_.worldFaces * 2 + 96);
  ImGui::Text("Scene draw calls: %d", 9 + (diagnostics_.effectPoints > 0 ? 1 : 0));
  ImGui::Text("Effect points: %d", diagnostics_.effectPoints);
  ImGui::SeparatorText("Shooting");
  ImGui::TextUnformatted("Left click: fire (mouse captured)");
  ImGui::TextUnformatted("Steve is the practice target");
  ImGui::TextWrapped("T: switch clear / wall-blocked demo and reset aim");
  ImGui::TextUnformatted(diagnostics_.targetInsideHut ?
      "Demo: target inside hut" : "Demo: target outside hut");
  ImGui::TextWrapped("One shot per click, 0.25 s cooldown, 50 unit range");
  ImGui::Text("Shots fired: %u", diagnostics_.shotsFired);
  if (diagnostics_.shotsFired == 0) {
    ImGui::TextUnformatted("Last shot: none");
  } else if (diagnostics_.lastShotHit) {
    const auto& hit = *diagnostics_.lastShotHit;
    ImGui::TextWrapped("Last shot: %s %zu at %.2f units",
        hit.kind == core::ShotHitKind::Target ? "target part" : "block",
        hit.index + 1, hit.distance);
    if (diagnostics_.lastShotObstructed) {
      ImGui::TextWrapped("Target behind wall: shot stopped by the nearer block");
    }
  } else {
    ImGui::TextUnformatted("Last shot: miss");
  }

  const ImVec2 size = ImGui::GetIO().DisplaySize;
  const ImVec2 centre(size.x * 0.5F, size.y * 0.5F);
  ImU32 colour = IM_COL32_WHITE;
  if (!interactionDisabled && diagnostics_.feedbackSeconds > 0.0F) {
    colour = !diagnostics_.lastShotHit ? IM_COL32(255, 100, 100, 255) :
        (diagnostics_.lastShotHit->kind == core::ShotHitKind::Target
            ? IM_COL32(100, 255, 130, 255) : IM_COL32(255, 190, 70, 255));
  }
  auto* foreground = ImGui::GetForegroundDrawList();
  foreground->AddLine(ImVec2(centre.x - 5, centre.y),
                      ImVec2(centre.x + 5, centre.y), colour);
  foreground->AddLine(ImVec2(centre.x, centre.y - 5),
                      ImVec2(centre.x, centre.y + 5), colour);
}

void CustomUi::setDiagnostics(const CameraDiagnostics& diagnostics) {
  diagnostics_ = diagnostics;
}

}  // namespace craft::app
