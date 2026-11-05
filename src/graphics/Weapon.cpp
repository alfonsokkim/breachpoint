#include "graphics/Weapon.h"

#include <cstddef>
#include <vector>

#include <glm/ext/matrix_transform.hpp>

#include "graphics/OpenGL.h"
#include "graphics/Voxel.h"

namespace craft::graphics {

Weapon::Weapon() {
  std::vector<VoxelVertex> vertices;
  for (const auto direction : kFaceDirections) {
    const auto face = makeVoxelFace(BlockType::Wood, direction, {0, 0, 0});
    for (const auto index : VoxelFace::kIndices) {
      vertices.push_back(face.vertices[index]);
    }
  }
  glGenVertexArrays(1, &vertexArray_);
  glGenBuffers(1, &vertexBuffer_);
  glBindVertexArray(vertexArray_);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VoxelVertex),
                vertices.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex),
                        reinterpret_cast<const void*>(offsetof(VoxelVertex, position)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex),
                        reinterpret_cast<const void*>(offsetof(VoxelVertex, uv)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex),
                        reinterpret_cast<const void*>(offsetof(VoxelVertex, normal)));
  glBindVertexArray(0);
}

Weapon::~Weapon() {
  glDeleteBuffers(1, &vertexBuffer_);
  glDeleteVertexArrays(1, &vertexArray_);
}

void Weapon::drawBox(const Shader& shader, const glm::mat4& model, glm::vec3 colour) const {
  shader.setInt("uTextured", 0);
  shader.setInt("uVertexColour", 0);
  shader.setVec3("uTint", colour);
  shader.setMat4("uModel", model);
  glBindVertexArray(vertexArray_);
  glDrawArrays(GL_TRIANGLES, 0, 36);
}

void Weapon::draw(const Shader& shader, const glm::mat4& frame) const {
  shader.setInt("uTextured", 0);
  shader.setVec3("uTint", {0.16F, 0.18F, 0.20F});
  shader.setFloat("uSpecular", 0.55F);
  shader.setFloat("uShininess", 64.0F);
  glBindVertexArray(vertexArray_);
  const auto box = [&](glm::vec3 minimum, glm::vec3 size) {
    shader.setMat4("uModel", glm::scale(glm::translate(frame, minimum), size));
    glDrawArrays(GL_TRIANGLES, 0, 36);
  };
  box({0.13F, -0.28F, -0.45F}, {0.14F, 0.12F, 0.28F});
  box({0.18F, -0.17F, -0.65F}, {0.04F, 0.04F, 0.25F});
}

}
