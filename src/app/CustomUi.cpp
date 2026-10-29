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
  ImGui::SeparatorText("Example bounds query");
  if (diagnostics_.exampleHitDistance) {
    ImGui::Text("Hit: %.2f world units", *diagnostics_.exampleHitDistance);
  } else {
    ImGui::TextUnformatted("Hit: none");
  }
  ImGui::TextUnformatted("AABB only, not a triangle hit");
  ImGui::TextUnformatted("Example: 4 faces, 4 triangles");
  ImGui::TextUnformatted("Scene draw calls: 1");

  const ImVec2 size = ImGui::GetIO().DisplaySize;
  const ImVec2 centre(size.x * 0.5F, size.y * 0.5F);
  auto* foreground = ImGui::GetForegroundDrawList();
  foreground->AddLine(ImVec2(centre.x - 5, centre.y),
                      ImVec2(centre.x + 5, centre.y), IM_COL32_WHITE);
  foreground->AddLine(ImVec2(centre.x, centre.y - 5),
                      ImVec2(centre.x, centre.y + 5), IM_COL32_WHITE);
}

void CustomUi::setDiagnostics(const CameraDiagnostics& diagnostics) {
  diagnostics_ = diagnostics;
}

}  // namespace craft::app
