#pragma once

#include <array>
#include <vector>

#include <glm/mat4x4.hpp>

#include "graphics/Texture.h"
#include "graphics/Voxel.h"
#include "core/Ray.h"
#include "core/TrainingGround.h"

namespace craft::graphics {

// A fixed-size cube of blocks drawn as one indexed mesh. Only faces that
// border air are meshed, so buried faces cost nothing to draw.
// A valid OpenGL context must exist for the lifetime of each Chunk.
class Chunk {
 public:
  static constexpr int kSize = 32;

  // `atlas` is shared between chunks and must outlive this chunk.
  explicit Chunk(const Texture& atlas);
  ~Chunk();

  Chunk(const Chunk&) = delete;
  Chunk& operator=(const Chunk&) = delete;

  // Places the chunk in the world; block (x, y, z) occupies [x, x+1] etc.
  [[nodiscard]] glm::mat4 getModel() const;

  // Binds the atlas to texture unit 0 and draws the mesh.
  void draw() const;
  void load(const core::TrainingGround& ground);

  [[nodiscard]] int getFaceCount() const noexcept { return faceCount_; }

  // World-space boxes from the same blocks used to build the visible mesh.
  [[nodiscard]] const std::vector<core::Aabb>& getSolidBounds() const noexcept {
    return solidBounds_;
  }

 private:
  using Blocks =
      std::array<std::array<std::array<BlockType, kSize>, kSize>, kSize>;

  // Builds the mesh from blocks_ and uploads it to the GPU.
  void buildMesh();

  // Cells outside the chunk count as empty, so edge faces are drawn.
  [[nodiscard]] bool isEmpty(int x, int y, int z) const;

  const Texture& atlas_;
  Blocks blocks_{};
  std::vector<core::Aabb> solidBounds_;
  unsigned int vertexArray_ = 0;
  unsigned int vertexBuffer_ = 0;
  unsigned int indexBuffer_ = 0;
  int indexCount_ = 0;
  int faceCount_ = 0;
};

}  // namespace craft::graphics
