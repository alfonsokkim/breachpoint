#include "graphics/PointEffects.h"

#include <cstddef>

#include "core/AssetPath.h"
#include "graphics/OpenGL.h"

namespace craft::graphics {

PointEffects::PointEffects()
    : shader_(core::assetPath("shaders/effects.vert"),
               core::assetPath("shaders/effects.frag")) {
  glGetFloatv(GL_POINT_SIZE_RANGE, &sizeRange_.x);
  glGenVertexArrays(1, &vertexArray_);
  glGenBuffers(1, &vertexBuffer_);
  glBindVertexArray(vertexArray_);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(EffectPoint),
                        reinterpret_cast<const void*>(offsetof(EffectPoint, position)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(EffectPoint),
                        reinterpret_cast<const void*>(offsetof(EffectPoint, colour)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(EffectPoint),
                        reinterpret_cast<const void*>(offsetof(EffectPoint, size)));
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(EffectPoint),
                        reinterpret_cast<const void*>(offsetof(EffectPoint, flash)));
  glBindVertexArray(0);
}

PointEffects::~PointEffects() {
  glDeleteBuffers(1, &vertexBuffer_);
  glDeleteVertexArrays(1, &vertexArray_);
}

void PointEffects::draw(const std::vector<EffectPoint>& points,
                        const glm::mat4& viewProjection,
                        int framebufferHeight) const {
  if (points.empty()) return;
  const bool blended = glIsEnabled(GL_BLEND);
  const bool sized = glIsEnabled(GL_PROGRAM_POINT_SIZE);
  GLboolean depthMask;
  GLint srcRgb, dstRgb, srcAlpha, dstAlpha;
  glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
  glGetIntegerv(GL_BLEND_SRC_RGB, &srcRgb);
  glGetIntegerv(GL_BLEND_DST_RGB, &dstRgb);
  glGetIntegerv(GL_BLEND_SRC_ALPHA, &srcAlpha);
  glGetIntegerv(GL_BLEND_DST_ALPHA, &dstAlpha);
  glEnable(GL_BLEND);
  glEnable(GL_PROGRAM_POINT_SIZE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthMask(GL_FALSE);
  shader_.use();
  shader_.setMat4("uViewProjection", viewProjection);
  shader_.setFloat("uPixelScale", framebufferHeight * 0.96049F);
  shader_.setVec2("uSizeRange", sizeRange_);
  glBindVertexArray(vertexArray_);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
  glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(EffectPoint),
                points.data(), GL_STREAM_DRAW);
  glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(points.size()));
  glDepthMask(depthMask);
  glBlendFuncSeparate(srcRgb, dstRgb, srcAlpha, dstAlpha);
  if (!blended) glDisable(GL_BLEND);
  if (!sized) glDisable(GL_PROGRAM_POINT_SIZE);
}

}
