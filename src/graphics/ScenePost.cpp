#include "graphics/ScenePost.h"

#include <stdexcept>
#include "core/AssetPath.h"
#include "graphics/OpenGL.h"

namespace craft::graphics {

ScenePost::ScenePost() : shader_(core::assetPath("shaders/post.vert"), core::assetPath("shaders/post.frag")) {
  glGenFramebuffers(1, &framebuffer_); glGenTextures(1, &colour_);
  glGenRenderbuffers(1, &depth_); glGenVertexArrays(1, &vertexArray_);
}

ScenePost::~ScenePost() {
  glDeleteFramebuffers(1, &framebuffer_); glDeleteTextures(1, &colour_);
  glDeleteRenderbuffers(1, &depth_); glDeleteVertexArrays(1, &vertexArray_);
}

void ScenePost::begin(int width, int height) {
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
  if (width != width_ || height != height_) {
    width_ = width; height_ = height;
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, colour_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colour_, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      throw std::runtime_error("Scene framebuffer is incomplete");
  }
  glViewport(0, 0, width, height);
  glEnable(GL_DEPTH_TEST); glDepthMask(GL_TRUE);
  glClearColor(0, 0, 0, 0); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ScenePost::present() const {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, width_, height_);
  glDisable(GL_DEPTH_TEST); glDisable(GL_CULL_FACE);
  shader_.use(); shader_.setInt("uScene", 0);
  glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, colour_);
  glBindVertexArray(vertexArray_); glDrawArrays(GL_TRIANGLES, 0, 3);
  glEnable(GL_DEPTH_TEST); glEnable(GL_CULL_FACE);
}

}
