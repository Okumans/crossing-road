#pragma once

#include "graphics/camera.hpp"
#include "graphics/mesh.hpp"
#include "graphics/shader.hpp"
#include "graphics/texture.hpp"
#include <glad/gl.h>

class Skybox : public IDrawable {
public:
  Skybox();
  ~Skybox();

  void draw(const Camera &camera, Shader &shader, const Texture &skyboxTex);
  GLuint getVAO() const { return m_vao; }

private:
  GLuint m_vao, m_vbo;
  void _setupSkybox();
};
