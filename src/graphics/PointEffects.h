#pragma once

#include <vector>

#include <glm/vec4.hpp>

#include "graphics/Shader.h"

namespace craft::graphics {

struct EffectPoint {
  glm::vec3 position;
  glm::vec4 colour;
  float size;
  float flash;
};

// Batches fading point sprites with depth testing and no depth writes.
class PointEffects {
 public:
  PointEffects();
  ~PointEffects();
  PointEffects(const PointEffects&) = delete;
  PointEffects& operator=(const PointEffects&) = delete;
  void draw(const std::vector<EffectPoint>& points, const glm::mat4& viewProjection,
             int framebufferHeight) const;

 private:
  Shader shader_;
  unsigned int vertexArray_ = 0;
  unsigned int vertexBuffer_ = 0;
  glm::vec2 sizeRange_{};
};

}
