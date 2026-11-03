#include "graphics/Voxel.h"

#include <cstddef>

namespace craft::graphics {

namespace {

constexpr float kAtlasTiles = 16.0F;

// Corners are listed bottom-left, top-left, top-right, bottom-right as seen
// from outside the block, which keeps every face's UV layout identical.
struct FaceTemplate {
  std::array<glm::vec3, 4> corners;
  glm::ivec3 normal;
};

// Indexed by FaceDirection.
constexpr std::array<FaceTemplate, 6> kFaceTemplates = {{
    // clang-format off
    {{{{0, 0, 1}, {0, 1, 1}, {1, 1, 1}, {1, 0, 1}}}, { 0,  0,  1}},  // Front
    {{{{1, 0, 1}, {1, 1, 1}, {1, 1, 0}, {1, 0, 0}}}, { 1,  0,  0}},  // Right
    {{{{1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {0, 0, 0}}}, { 0,  0, -1}},  // Back
    {{{{0, 0, 0}, {0, 1, 0}, {0, 1, 1}, {0, 0, 1}}}, {-1,  0,  0}},  // Left
    {{{{0, 1, 1}, {0, 1, 0}, {1, 1, 0}, {1, 1, 1}}}, { 0,  1,  0}},  // Top
    {{{{0, 0, 0}, {0, 0, 1}, {1, 0, 1}, {1, 0, 0}}}, { 0, -1,  0}},  // Bottom
    // clang-format on
}};

const FaceTemplate& faceTemplate(FaceDirection direction) {
  return kFaceTemplates[static_cast<std::size_t>(direction)];
}

// Atlas tile as (column, row), counted from the image's top-left corner.
glm::ivec2 atlasTile(BlockType type, FaceDirection direction) {
  switch (type) {
    case BlockType::Grass:
      if (direction == FaceDirection::Top) {
        return {0, 13};
      }
      if (direction == FaceDirection::Bottom) {
        return {0, 15};
      }
      return {0, 14};
    case BlockType::Cobblestone:
      return {5, 15};
    case BlockType::Wood:
      return {7, 15};
    case BlockType::Air:
    case BlockType::Dirt:
      break;
  }
  return {0, 15};
}

}  // namespace

glm::ivec3 faceNeighbourOffset(FaceDirection direction) {
  return faceTemplate(direction).normal;
}

const std::array<glm::vec3, 4>& faceCorners(FaceDirection direction) {
  return faceTemplate(direction).corners;
}

VoxelFace makeVoxelFace(BlockType type, FaceDirection direction,
                        const glm::ivec3& position) {
  const FaceTemplate& face = faceTemplate(direction);
  const glm::ivec2 tile = atlasTile(type, direction);

  // Texture v = 0 is the top of the image, so a tile's top edge has the
  // smaller v.
  const float left = static_cast<float>(tile.x) / kAtlasTiles;
  const float right = static_cast<float>(tile.x + 1) / kAtlasTiles;
  const float top = static_cast<float>(tile.y) / kAtlasTiles;
  const float bottom = static_cast<float>(tile.y + 1) / kAtlasTiles;
  const std::array<glm::vec2, 4> uvs = {{
      {left, bottom},
      {left, top},
      {right, top},
      {right, bottom},
  }};

  const glm::vec3 offset(position);
  const glm::vec3 normal(face.normal);
  VoxelFace result{};
  for (std::size_t i = 0; i < result.vertices.size(); ++i) {
    result.vertices[i] = {face.corners[i] + offset, uvs[i], normal};
  }
  return result;
}

}  // namespace craft::graphics
