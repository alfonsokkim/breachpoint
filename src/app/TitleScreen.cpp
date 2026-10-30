#include "app/TitleScreen.h"

#include <algorithm>
#include <string_view>

#include <glm/vec4.hpp>

#include "core/AssetPath.h"
#include "graphics/OpenGL.h"
#include "graphics/Window.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace craft::app {

namespace {

constexpr std::string_view kTitle = "Breachpoint";
constexpr std::string_view kPrompt = "Press Enter to start";
constexpr float kLetterSpacing = 0.08F;

float fittedGlyphWidth(std::string_view text, float size, float width,
                       float height) {
  const float lineWidth = graphics::TextRenderer::measure(text, 1.0F,
                                                         kLetterSpacing);
  return std::min(0.5F * size * height, 0.85F * width / lineWidth);
}

}

TitleScreen::TitleScreen(graphics::Window& window)
    : window_(window),
      text_(core::assetPath("textures/font.png"),
            core::assetPath("shaders/text.vert"),
            core::assetPath("shaders/text.frag")) {}

void TitleScreen::update() {
  GLFWwindow* handle = window_.nativeHandle();
  const bool enterDown = glfwGetKey(handle, GLFW_KEY_ENTER) == GLFW_PRESS ||
                         glfwGetKey(handle, GLFW_KEY_KP_ENTER) == GLFW_PRESS;
  if (enterDown && !enterWasDown_) {
    finished_ = true;
  }
  enterWasDown_ = enterDown;
}

void TitleScreen::draw(int framebufferWidth, int framebufferHeight) {
  glViewport(0, 0, framebufferWidth, framebufferHeight);
  glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  const float width = static_cast<float>(framebufferWidth);
  const float height = static_cast<float>(framebufferHeight);
  text_.begin(framebufferWidth, framebufferHeight);

  const auto drawLine = [&](std::string_view line, float size, float centreY,
                            const glm::vec4& colour) {
    const float glyph = fittedGlyphWidth(line, size, width, height);
    const float lineWidth = graphics::TextRenderer::measure(line, glyph,
                                                           kLetterSpacing);
    text_.drawText(line, (width - lineWidth) * 0.5F, height * centreY - glyph,
                   glyph, colour, kLetterSpacing);
  };

  drawLine(kTitle, titleSize_, 0.44F, glm::vec4(1.0F));
  drawLine(kPrompt, promptSize_, 0.57F, glm::vec4(0.65F, 0.65F, 0.65F, 1.0F));
  text_.end();
}

void TitleScreen::setTextSizes(float titleSize, float promptSize) {
  titleSize_ = std::max(0.0F, titleSize);
  promptSize_ = std::max(0.0F, promptSize);
}

bool TitleScreen::finished() const noexcept {
  return finished_;
}

}
