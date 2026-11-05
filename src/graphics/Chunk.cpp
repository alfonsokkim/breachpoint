#include "graphics/Chunk.h"

#include <cstddef>
#include <vector>

#include "graphics/OpenGL.h"

namespace craft::graphics {

Chunk::Chunk(const Texture& atlas) : atlas_(atlas) {
  load(core::makeTrainingGround(0));
}

Chunk::~Chunk() {
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

glm::mat4 Chunk::getModel() const {
  return glm::mat4(1.0F);
}

void Chunk::draw() const {
  atlas_.bind(0);
  glBindVertexArray(vertexArray_);
  glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, nullptr);
}

void Chunk::load(const core::TrainingGround& ground) {
  blocks_ = {};
  constexpr BlockType materials[] = {BlockType::Concrete, BlockType::TrainingWall,
      BlockType::TrainingCover, BlockType::Yellow, BlockType::Red, BlockType::Blue};
  for (const auto& block : ground.blocks) {
    const auto p = block.cell;
    if (p.x >= 0 && p.x < kSize && p.y >= 0 && p.y < kSize && p.z >= 0 && p.z < kSize)
      blocks_[p.x][p.y][p.z] = materials[static_cast<int>(block.material)];
  }
  buildMesh();
}

void Chunk::buildMesh() {
  std::vector<VoxelVertex> vertices;
  std::vector<unsigned int> indices;
  solidBounds_.clear();

  for (int x = 0; x < kSize; ++x) {
    for (int y = 0; y < kSize; ++y) {
      for (int z = 0; z < kSize; ++z) {
        const BlockType type = blocks_[x][y][z];
        if (type == BlockType::Air) {
          continue;
        }
        const glm::ivec3 position(x, y, z);
        const glm::vec3 minimum(position);
        solidBounds_.push_back({minimum, minimum + glm::vec3(1.0F)});
        for (const FaceDirection direction : kFaceDirections) {
          const glm::ivec3 neighbour =
              position + faceNeighbourOffset(direction);
          if (!isEmpty(neighbour.x, neighbour.y, neighbour.z)) {
            continue;
          }
          const auto firstVertex = static_cast<unsigned int>(vertices.size());
          const VoxelFace face = makeVoxelFace(type, direction, position);
          vertices.insert(vertices.end(), face.vertices.begin(),
                          face.vertices.end());
          for (const unsigned int index : VoxelFace::kIndices) {
            indices.push_back(firstVertex + index);
          }
        }
      }
    }
  }

  indexCount_ = static_cast<int>(indices.size());
  faceCount_ = indexCount_ / static_cast<int>(VoxelFace::kIndices.size());

  if (!vertexArray_) glGenVertexArrays(1, &vertexArray_);
  if (!vertexBuffer_) glGenBuffers(1, &vertexBuffer_);
  if (!indexBuffer_) glGenBuffers(1, &indexBuffer_);

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
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride,
                        reinterpret_cast<const void*>(offsetof(VoxelVertex, colour)));
  glBindVertexArray(0);
}

bool Chunk::isEmpty(int x, int y, int z) const {
  if (x < 0 || x >= kSize || y < 0 || y >= kSize || z < 0 || z >= kSize) {
    return true;
  }
  return blocks_[x][y][z] == BlockType::Air;
}

}  // namespace craft::graphics
