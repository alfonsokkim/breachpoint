#include "graphics/Chunk.h"

#include <cstddef>
#include <vector>

#include "graphics/OpenGL.h"

namespace craft::graphics {

Chunk::Chunk(const Texture& atlas) : atlas_(atlas) {
  seedDemoScene();
  buildMesh();
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

void Chunk::seedDemoScene() {
  // 12x12 platform: two layers of cobblestone, two of dirt, grass on top.
  for (int x = 10; x <= 21; ++x) {
    for (int z = 10; z <= 21; ++z) {
      for (int y = 0; y <= 4; ++y) {
        if (y <= 1) {
          blocks_[x][y][z] = BlockType::Cobblestone;
        } else if (y <= 3) {
          blocks_[x][y][z] = BlockType::Dirt;
        } else {
          blocks_[x][y][z] = BlockType::Grass;
        }
      }
    }
  }

  // 5x5 hollow hut with a roof and a two-block doorway on its -x wall.
  for (int x = 13; x <= 17; ++x) {
    for (int z = 13; z <= 17; ++z) {
      for (int y = 5; y <= 7; ++y) {
        const bool wall = x == 13 || x == 17 || z == 13 || z == 17;
        const bool roof = y == 7;
        const bool doorway = x == 13 && z == 15 && !roof;
        if ((wall || roof) && !doorway) {
          blocks_[x][y][z] = BlockType::Wood;
        }
      }
    }
  }
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

bool Chunk::isEmpty(int x, int y, int z) const {
  if (x < 0 || x >= kSize || y < 0 || y >= kSize || z < 0 || z >= kSize) {
    return true;
  }
  return blocks_[x][y][z] == BlockType::Air;
}

}  // namespace craft::graphics
