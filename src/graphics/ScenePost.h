#pragma once

#include "graphics/Shader.h"

namespace craft::graphics {

// HDR scene target and a fullscreen dusk/night-vision presentation pass.
class ScenePost {
 public:
  ScenePost();
  ~ScenePost();
  ScenePost(const ScenePost&) = delete;
  ScenePost& operator=(const ScenePost&) = delete;
  void begin(int width, int height);
  void present(bool nightVision, float seconds) const;

 private:
  Shader shader_;
  unsigned int framebuffer_ = 0, colour_ = 0, depth_ = 0, vertexArray_ = 0;
  int width_ = 0, height_ = 0;
};

}
