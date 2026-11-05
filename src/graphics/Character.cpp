#include "graphics/Character.h"

#include <vector>

#include <glm/ext/matrix_transform.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "graphics/OpenGL.h"
#include "graphics/Voxel.h"
#include "core/BoxTransform.h"

namespace craft::graphics {

namespace {

constexpr float kSkinSize = 64.0F;
constexpr float kPixelsPerBlock = 16.0F;
constexpr GLsizei kIndicesPerBox =
    static_cast<GLsizei>(kFaceDirections.size() * VoxelFace::kIndices.size());

// One box of the model, measured in skin pixels. `offset` is the box's
// minimum corner relative to the feet, and `skinOrigin` is the top-left
// corner of the box's unfolded faces in the skin.
struct PartLayout {
  glm::vec3 offset;
  glm::vec3 size;
  glm::vec2 skinOrigin;
};

constexpr std::array<PartLayout, 6> kPartLayouts = {{
    // clang-format off
    {{-4, 24, -4}, {8,  8, 8}, { 0,  0}},  // head
    {{-4, 12, -2}, {8, 12, 4}, {16, 16}},  // torso
    {{ 4, 12, -2}, {4, 12, 4}, {32, 48}},  // left arm
    {{-8, 12, -2}, {4, 12, 4}, {40, 16}},  // right arm
    {{ 0,  0, -2}, {4, 12, 4}, {16, 48}},  // left leg
    {{-4,  0, -2}, {4, 12, 4}, { 0, 16}},  // right leg
    // clang-format on
}};

// Where one face of a w x h x d box sits in the skin, as (left, top, width,
// height) in pixels. Each box's faces are unfolded like this:
//
//                 [ top ][bottom]
//   [char right][ front ][char left][ back ]
//
// The character faces +z, so its own right side is the -x face.
glm::vec4 skinRect(FaceDirection direction, const PartLayout& layout) {
  const float w = layout.size.x;
  const float h = layout.size.y;
  const float d = layout.size.z;
  const glm::vec2 origin = layout.skinOrigin;
  switch (direction) {
    case FaceDirection::Front:
      return {origin.x + d, origin.y + d, w, h};
    case FaceDirection::Right:
      return {origin.x + d + w, origin.y + d, d, h};
    case FaceDirection::Back:
      return {origin.x + d + w + d, origin.y + d, w, h};
    case FaceDirection::Left:
      return {origin.x, origin.y + d, d, h};
    case FaceDirection::Top:
      return {origin.x + d, origin.y, w, d};
    case FaceDirection::Bottom:
      return {origin.x + d + w, origin.y, w, d};
  }
  return {};
}

// Appends the 24 vertices and 36 indices of one box, with corners at
// 0..size so the part's transform places it.
void appendBox(const PartLayout& layout, std::vector<VoxelVertex>& vertices,
               std::vector<unsigned int>& indices) {
  for (const FaceDirection direction : kFaceDirections) {
    const glm::vec4 rect = skinRect(direction, layout) / kSkinSize;
    const float left = rect.x;
    const float top = rect.y;
    const float right = rect.x + rect.z;
    const float bottom = rect.y + rect.w;

    // Corners run bottom-left, top-left, top-right, bottom-right. Skins store
    // the bottom face upside down relative to the top face.
    const std::array<glm::vec2, 4> uvs =
        direction == FaceDirection::Bottom
            ? std::array<glm::vec2, 4>{{
                  {left, top}, {left, bottom}, {right, bottom}, {right, top}}}
            : std::array<glm::vec2, 4>{{
                  {left, bottom}, {left, top}, {right, top}, {right, bottom}}};

    const auto firstVertex = static_cast<unsigned int>(vertices.size());
    const glm::vec3 normal(faceNeighbourOffset(direction));
    const auto& corners = faceCorners(direction);
    for (std::size_t i = 0; i < corners.size(); ++i) {
      vertices.push_back({corners[i] * layout.size, uvs[i], normal});
    }
    for (const unsigned int index : VoxelFace::kIndices) {
      indices.push_back(firstVertex + index);
    }
  }
}

}  // namespace

Character::Character(const Texture& skin, const glm::vec3& feetPosition)
    : skin_(skin),
      model_(glm::scale(glm::translate(glm::mat4(1.0F), feetPosition),
                        glm::vec3(1.0F / kPixelsPerBlock))) {
  static_assert(kPartLayouts.size() == kPartCount);

  std::vector<VoxelVertex> vertices;
  std::vector<unsigned int> indices;
  for (std::size_t i = 0; i < kPartCount; ++i) {
    parts_[i] = {glm::translate(glm::mat4(1.0F), kPartLayouts[i].offset),
                 static_cast<unsigned int>(indices.size())};
    appendBox(kPartLayouts[i], vertices, indices);
  }
  updateHitBounds();

  glGenVertexArrays(1, &vertexArray_);
  glGenBuffers(1, &vertexBuffer_);
  glGenBuffers(1, &indexBuffer_);

  glBindVertexArray(vertexArray_);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
  glBufferData(GL_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(vertices.size() * sizeof(VoxelVertex)),
               vertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
               indices.data(), GL_STATIC_DRAW);

  constexpr GLsizei stride = sizeof(VoxelVertex);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                        reinterpret_cast<const void*>(
                            offsetof(VoxelVertex, position)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
                        reinterpret_cast<const void*>(offsetof(VoxelVertex, uv)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
                        reinterpret_cast<const void*>(
                            offsetof(VoxelVertex, normal)));
  glBindVertexArray(0);
}

Character::~Character() {
  if (vertexBuffer_ != 0) {
    glDeleteBuffers(1, &vertexBuffer_);
  }
  if (indexBuffer_ != 0) {
    glDeleteBuffers(1, &indexBuffer_);
  }
  if (vertexArray_ != 0) {
    glDeleteVertexArrays(1, &vertexArray_);
  }
}

void Character::setFeetPosition(const glm::vec3& feetPosition) {
  model_ = glm::scale(glm::translate(glm::mat4(1.0F), feetPosition),
                      glm::vec3(1.0F / kPixelsPerBlock));
  updateHitBounds();
}

void Character::updateHitBounds() {
  hitBounds_.clear();
  hitBounds_.reserve(kPartCount);
  for (std::size_t i = 0; i < kPartCount; ++i) {
    hitBounds_.push_back(core::transformBounds(
        {glm::vec3(0), kPartLayouts[i].size}, model_ * parts_[i].model));
  }
}

void Character::draw(const Shader& shader) const {
  skin_.bind(0);
  glBindVertexArray(vertexArray_);
  for (const Part& part : parts_) {
    shader.setMat4("uModel", model_ * part.model);
    glDrawElements(GL_TRIANGLES, kIndicesPerBox, GL_UNSIGNED_INT,
                   reinterpret_cast<const void*>(part.firstIndex *
                                                 sizeof(unsigned int)));
  }
}

}  // namespace craft::graphics
