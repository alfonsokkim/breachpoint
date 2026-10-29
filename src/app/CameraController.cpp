#include "app/CameraController.h"

#include "core/Camera.h"
#include "graphics/Window.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace craft::app {

CameraController::CameraController(graphics::Window& window, core::Camera& camera)
    : window_(window), camera_(camera) {}

CameraController::~CameraController() { releaseMouse(); }

void CameraController::setCaptured(bool captured) {
  if (captured_ != captured) {
    captured_ = captured;
    haveMouseSample_ = false;
    glfwSetInputMode(window_.nativeHandle(), GLFW_CURSOR,
                     captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
  }
}

void CameraController::releaseMouse() { setCaptured(false); }
bool CameraController::mouseCaptured() const noexcept { return captured_; }

void CameraController::update(float deltaSeconds, bool enabled,
                               bool keyboardBlocked) {
  GLFWwindow* handle = window_.nativeHandle();
  const bool tabDown = glfwGetKey(handle, GLFW_KEY_TAB) == GLFW_PRESS;
  const bool focused = glfwGetWindowAttrib(handle, GLFW_FOCUSED) == GLFW_TRUE;
  if (!enabled || !focused) {
    releaseMouse();
    tabWasDown_ = tabDown;
    return;
  }
  if (tabDown && !tabWasDown_ && !keyboardBlocked) {
    setCaptured(!captured_);
  }
  tabWasDown_ = tabDown;
  if (!captured_) {
    return;
  }

  double x = 0.0;
  double y = 0.0;
  glfwGetCursorPos(handle, &x, &y);
  if (haveMouseSample_) {
    camera_.look(static_cast<float>((x - previousX_) * 0.1),
                  static_cast<float>((previousY_ - y) * 0.1));
  }
  previousX_ = x;
  previousY_ = y;
  haveMouseSample_ = true;

  if (!keyboardBlocked) {
    const auto down = [handle](int key) {
      return glfwGetKey(handle, key) == GLFW_PRESS ? 1.0F : 0.0F;
    };
    camera_.move({down(GLFW_KEY_D) - down(GLFW_KEY_A),
                   down(GLFW_KEY_W) - down(GLFW_KEY_S)}, deltaSeconds);
  }
}

}
