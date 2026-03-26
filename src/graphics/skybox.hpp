#pragma once

#include <glad/gl.h>
#include <memory>
#include "graphics/shader.hpp"
#include "graphics/camera.hpp"
#include "graphics/texture.hpp"

class Skybox {
public:
    Skybox();
    ~Skybox();

    void draw(const Camera& camera, Shader& shader, const Texture& skyboxTex);

private:
    GLuint m_vao, m_vbo;
    void _setupSkybox();
};
