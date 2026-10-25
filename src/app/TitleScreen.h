#pragma once

#include "graphics/TextRenderer.h"

namespace craft::graphics {
class Window;
}

namespace craft::app {

// Shows the title and an Enter prompt on a plain black background.
class TitleScreen {
 public:
  explicit TitleScreen(graphics::Window& window);

  TitleScreen(const TitleScreen&) = delete;
  TitleScreen& operator=(const TitleScreen&) = delete;

  // Reads the Enter key once per frame.
  void update();

  // Draws the whole frame, including clearing the framebuffer.
  void draw(int framebufferWidth, int framebufferHeight);

  // Sets text cell heights as fractions of the window height.
  void setTextSizes(float titleSize, float promptSize);

  // True once Enter has been pressed and the game should start.
  [[nodiscard]] bool finished() const noexcept;

 private:
  graphics::Window& window_;
  graphics::TextRenderer text_;
  float titleSize_ = 0.12F;
  float promptSize_ = 0.04F;
  bool finished_ = false;
  bool enterWasDown_ = true;
};

}
