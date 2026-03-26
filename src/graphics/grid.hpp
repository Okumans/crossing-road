#pragma once

#include "graphics/shader.hpp"
#include "graphics/camera.hpp"
#include <glad/gl.h>

class Grid {
public:
    Grid();
    ~Grid();
    
    void draw(const Camera& camera, Shader& shader);

private:
    GLuint m_vao, m_vbo;
    void _setupGrid();
};
