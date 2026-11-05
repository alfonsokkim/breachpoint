#include "graphics/SpotShadows.h"

#include <stdexcept>
#include <string>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/trigonometric.hpp>
#include "core/AssetPath.h"
#include "graphics/OpenGL.h"

namespace craft::graphics {

SpotShadows::SpotShadows(const Chunk& chunk, const core::TrainingGround& ground) {
  const bool culled = glIsEnabled(GL_CULL_FACE);
  const bool depthTested = glIsEnabled(GL_DEPTH_TEST);
  const bool offset = glIsEnabled(GL_POLYGON_OFFSET_FILL);
  GLint oldFramebuffer, oldProgram, viewport[4];
  GLfloat oldFactor, oldUnits;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFramebuffer);
  glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
  glGetIntegerv(GL_VIEWPORT, viewport);
  glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &oldFactor);
  glGetFloatv(GL_POLYGON_OFFSET_UNITS, &oldUnits);
  Shader depth(core::assetPath("shaders/shadow.vert"), core::assetPath("shaders/shadow.frag"));
  glGenTextures(1, &texture_); glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D_ARRAY, texture_);
  glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT24, 1024, 1024, 4, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  const float border[] = {1, 1, 1, 1};
  glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, border);
  unsigned int framebuffer;
  glGenFramebuffers(1, &framebuffer); glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glDrawBuffer(GL_NONE); glReadBuffer(GL_NONE);
  glViewport(0, 0, 1024, 1024); glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE); glEnable(GL_POLYGON_OFFSET_FILL); glPolygonOffset(2, 4);
  depth.use(); depth.setMat4("uModel", chunk.getModel());
  for (std::size_t i = 0; i < matrices_.size(); ++i) {
    const auto& light = ground.lights.at(i);
    matrices_[i] = glm::perspective(glm::radians(76.0F), 1.0F, 0.25F, 50.0F) *
        glm::lookAt(light.position, light.aim, glm::vec3(0, 1, 0));
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture_, 0, static_cast<int>(i));
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      throw std::runtime_error("Floodlight shadow framebuffer is incomplete");
    glClear(GL_DEPTH_BUFFER_BIT);
    depth.setMat4("uLightViewProjection", matrices_[i]); chunk.draw();
  }
  glPolygonOffset(oldFactor, oldUnits);
  if (offset) glEnable(GL_POLYGON_OFFSET_FILL); else glDisable(GL_POLYGON_OFFSET_FILL);
  if (culled) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
  if (depthTested) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
  glBindFramebuffer(GL_FRAMEBUFFER, oldFramebuffer); glDeleteFramebuffers(1, &framebuffer);
  glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
  glUseProgram(oldProgram);
  glActiveTexture(GL_TEXTURE0);
}

SpotShadows::~SpotShadows() { glDeleteTextures(1, &texture_); }

void SpotShadows::bind(const Shader& shader) const {
  glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D_ARRAY, texture_);
  shader.setInt("uShadowMap", 3);
  for (int i = 0; i < 4; ++i) shader.setMat4("uShadowMatrices[" + std::to_string(i) + "]", matrices_[i]);
  glActiveTexture(GL_TEXTURE0);
}

}
