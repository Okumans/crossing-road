#pragma once

#include "graphics/shader.hpp"
#include "resource/texture_manager.hpp"
#include <glad/gl.h>
#include <glm/glm.hpp>

enum class RowType { GRASS, ROAD, WATER };

class Row {
public:
    Row(float zPos, RowType type);
    ~Row();

    void draw(Shader& shader);
    float getZ() const { return m_zPos; }
    RowType getType() const { return m_type; }

private:
    float m_zPos;
    RowType m_type;
    GLuint m_vao, m_vbo;
    
    void _setupMesh();
    std::shared_ptr<Texture> _getTexture();
};
