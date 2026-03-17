#pragma once

#include "shader.h"
#include <memory>
#include <unordered_map>

enum class ShaderType { UI };

class ShaderManager {
public:
  static std::unordered_map<ShaderType, std::unique_ptr<Shader>> shaders;

  static Shader &loadShader(ShaderType type, const char *vertShaderPath,
                            const char *fragShaderPath);

  static Shader &loadShaderSource(ShaderType type, const char *vertShaderSource,
                                  const char *fragShaderSource);

  static Shader &getShader(ShaderType type);
};
