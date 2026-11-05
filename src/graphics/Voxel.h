#pragma once

#include <array>
#include <cstdint>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace craft::graphics {

// What occupies one cell of a chunk. Air is empty space and is never drawn.
enum class BlockType : std::uint8_t {
  Air,
  Grass,
  Dirt,
  Cobblestone,
  Wood,
  Concrete,
  TrainingWall,
  TrainingCover,
  Yellow,
  Red,
  Blue,
};

// The six sides of a block, named by the axis their outward normal points to.
enum class FaceDirection {
  Front,   // +z
  Right,   // +x
  Back,    // -z
  Left,    // -x
  Top,     // +y
  Bottom,  // -y
};

inline constexpr std::array<FaceDirection, 6> kFaceDirections = {
    FaceDirection::Front, FaceDirection::Right, FaceDirection::Back,
    FaceDirection::Left,  FaceDirection::Top,   FaceDirection::Bottom,
};

// One mesh vertex, laid out to match the chunk shader's vertex attributes.
struct VoxelVertex {
  glm::vec3 position;
  glm::vec2 uv;
  glm::vec3 normal;
  glm::vec3 colour{1.0F};
};

// A single square block face, split into two counter-clockwise triangles.
struct VoxelFace {
  std::array<VoxelVertex, 4> vertices;
  static constexpr std::array<unsigned int, 6> kIndices = {1, 0, 3, 1, 3, 2};
};

// Offset to the neighbouring cell that this face touches.
glm::ivec3 faceNeighbourOffset(FaceDirection direction);

// Corners of one face of the unit cube, listed bottom-left, top-left,
// top-right, bottom-right as seen from outside, matching VoxelFace::kIndices.
const std::array<glm::vec3, 4>& faceCorners(FaceDirection direction);

// Builds one face of a unit block whose minimum corner sits at `position`,
// with UVs pointing at the block's tile in the 16x16 texture atlas.
VoxelFace makeVoxelFace(BlockType type, FaceDirection direction,
                        const glm::ivec3& position);

}  // namespace craft::graphics
