#pragma once

#include "graphics/shader.hpp"
#include <memory>
#include <unordered_map>

enum class ShaderType { UI, PBR, SKYBOX, SHADOW, IRRADIANCE, WATER, DEBUG };

class ShaderManager {
public:
  static std::unordered_map<ShaderType, std::unique_ptr<Shader>> shaders;

  static Shader &loadShader(ShaderType type, const char *vertShaderPath,
                            const char *fragShaderPath);

  static Shader &loadShaderSource(ShaderType type, const char *vertShaderSource,
                                  const char *fragShaderSource);

  static Shader &getShader(ShaderType type);
};
