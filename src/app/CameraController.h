#pragma once

namespace craft::core {
class Camera;
}

namespace craft::graphics {
class Window;
}

namespace craft::app {

class CameraController {
 public:
  CameraController(graphics::Window& window, core::Camera& camera);
  ~CameraController();
  CameraController(const CameraController&) = delete;
  CameraController& operator=(const CameraController&) = delete;

  // Reads GLFW input while the scene is active and freezes during benchmarks.
  void update(float deltaSeconds, bool enabled, bool keyboardBlocked);
  void releaseMouse();
  [[nodiscard]] bool mouseCaptured() const noexcept;

 private:
  void setCaptured(bool captured);

  graphics::Window& window_;
  core::Camera& camera_;
  bool captured_ = false;
  bool tabWasDown_ = true;
  bool haveMouseSample_ = false;
  double previousX_ = 0.0;
  double previousY_ = 0.0;
};

}
