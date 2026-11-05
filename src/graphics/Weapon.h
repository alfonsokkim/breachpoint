#pragma once

#include <glm/mat4x4.hpp>

#include "graphics/Shader.h"

namespace craft::graphics {

// A simple body and barrel using the chunk vertex layout. Requires a GL context.
class Weapon {
 public:
  Weapon();
  ~Weapon();
  Weapon(const Weapon&) = delete;
  Weapon& operator=(const Weapon&) = delete;
  void draw(const Shader& shader, const glm::mat4& frame) const;

 private:
  unsigned int vertexArray_ = 0;
  unsigned int vertexBuffer_ = 0;
};

}
