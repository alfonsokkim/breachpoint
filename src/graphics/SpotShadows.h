#pragma once

#include <array>
#include "graphics/Chunk.h"
#include "graphics/Shader.h"

namespace craft::graphics {

// Static courtyard shadows for the four fixed floodlights, on texture unit 3.
class SpotShadows {
 public:
  SpotShadows(const Chunk& chunk, const core::TrainingGround& ground);
  ~SpotShadows();
  SpotShadows(const SpotShadows&) = delete;
  SpotShadows& operator=(const SpotShadows&) = delete;
  void bind(const Shader& shader) const;

 private:
  unsigned int texture_ = 0;
  std::array<glm::mat4, 4> matrices_{};
};

}
