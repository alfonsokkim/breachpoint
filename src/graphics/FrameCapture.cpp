#include "graphics/FrameCapture.h"

#include <vector>
#include <stb_image_write.h>
#include "graphics/OpenGL.h"

namespace craft::graphics {

bool saveFrame(const char* path, int width, int height) {
  if (!path || width <= 0 || height <= 0) return false;
  std::vector<unsigned char> pixels(static_cast<std::size_t>(width) * height * 4);
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
  stbi_flip_vertically_on_write(1);
  const bool saved = stbi_write_png(path, width, height, 4, pixels.data(), width * 4) != 0;
  stbi_flip_vertically_on_write(0);
  return saved;
}

}
