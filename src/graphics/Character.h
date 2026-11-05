#pragma once

#include <array>
#include <cstddef>
#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "core/Ray.h"

namespace craft::graphics {

// A Minecraft-style humanoid built from six textured boxes. Each body part
// keeps its own transform so parts can be animated or hit-tested separately.
// The model faces +z, with its left side towards +x.
// A valid OpenGL context must exist for the lifetime of each Character.
class Character {
 public:
  // `skin` uses the 64x64 Minecraft skin layout and must outlive this
  // character. `feetPosition` is the point on the ground between the feet.
  Character(const Texture& skin, const glm::vec3& feetPosition);
  ~Character();

  Character(const Character&) = delete;
  Character& operator=(const Character&) = delete;

  // Binds the skin to texture unit 0 and draws each part, setting the
  // shader's uModel uniform per part.
  void draw(const Shader& shader) const;

  // Part bounds from the same model transforms used by draw().
  [[nodiscard]] const std::vector<core::Aabb>& getHitBounds() const noexcept {
    return hitBounds_;
  }

 private:
  struct Part {
    glm::mat4 model;
    unsigned int firstIndex;
  };

  static constexpr std::size_t kPartCount = 6;

  void updateHitBounds();

  const Texture& skin_;
  glm::mat4 model_;
  std::array<Part, kPartCount> parts_{};
  std::vector<core::Aabb> hitBounds_;
  unsigned int vertexArray_ = 0;
  unsigned int vertexBuffer_ = 0;
  unsigned int indexBuffer_ = 0;
};

}  // namespace craft::graphics
