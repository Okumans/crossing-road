#pragma once

#include "graphics/material.hpp"
#include "graphics/texture.hpp"
#include "resource/material_manager.hpp"
#include "resource/texture_manager.hpp"

#include <concepts>
#include <filesystem>
#include <format>
#include <random>
#include <string>

inline void loadMaterialFolder(const std::string &materialName,
                               const std::string &folderPath) {
  namespace fs = std::filesystem;

  const std::vector<std::pair<std::string, TextureType>> textureTypes = {
      {"diffuse", TextureType::DIFFUSE},
      {"height", TextureType::HEIGHT},
      {"normal", TextureType::NORMAL},
      {"ao", TextureType::AO},
      {"roughness", TextureType::ROUGHNESS},
      {"metallic", TextureType::METALLIC}};

  auto builder = Material::builder();
  bool foundAny = false;

  for (const auto &[suffix, type] : textureTypes) {
    std::string baseFile = folderPath + "/" + suffix;
    std::string fullPath;

    if (fs::exists(baseFile + ".jpg")) {
      fullPath = baseFile + ".jpg";
    } else if (fs::exists(baseFile + ".png")) {
      fullPath = baseFile + ".png";
    } else {
      continue;
    }

    auto tex = TextureManager::loadTexture(
        TextureName(std::format("{}_{}", materialName, suffix)), type,
        fullPath.c_str());

    foundAny = true;
    switch (type) {
    case TextureType::DIFFUSE:
      builder.setDiffuse(tex);
      break;
    case TextureType::HEIGHT:
      builder.setHeight(tex);
      break;
    case TextureType::NORMAL:
      builder.setNormal(tex);
      break;
    case TextureType::AO:
      builder.setAO(tex);
      break;
    case TextureType::ROUGHNESS:
      builder.setRoughness(tex);
      break;
    case TextureType::METALLIC:
      builder.setMetallic(tex);
      break;
    default:
      break;
    }
  }

  if (foundAny) {
    MaterialManager::addMaterial(materialName, builder.create());
  }
}

class Random {
public:
  Random() = delete;

  /**
   * @brief Generates a random integer in range [min, max]
   * Constrained to integral types (int, size_t, uint32_t, etc.)
   */
  template <std::integral T = int> static T randInt(T min, T max) {
    std::uniform_int_distribution<T> dist(min, max);
    return dist(s_engine);
  }

  /**
   * @brief Generates a random float in range [min, max]
   * Constrained to floating point types (float, double, long double)
   */
  template <std::floating_point T = float> static T randFloat(T min, T max) {
    std::uniform_real_distribution<T> dist(min, max);
    return dist(s_engine);
  }

  /**
   * @brief Generates a random integer in range [min, max] using custom weights.
   * The weights vector size must equal (max - min + 1).
   */
  template <std::integral T = int>
  static T randWeighted(T min, T max, const std::vector<double> &weights) {
    // weights[0] corresponds to min, weights[1] to min + 1, etc.
    std::discrete_distribution<T> dist(weights.begin(), weights.end());
    return min + dist(s_engine);
  }

  /**
   * @brief Returns true based on a probability P where 0 <= P <= 1
   */
  static bool randChance(std::floating_point auto prob) {
    return randFloat<decltype(prob)>(0, 1) < prob;
  }

  static void setSeed(unsigned int seed) { s_engine.seed(seed); }

private:
  // Using mt19937_64 for better compatibility with 64-bit types like size_t
  inline static std::mt19937_64 s_engine{std::random_device{}()};
};

template <typename T, std::invocable<T &> F>
constexpr T withBase(T base, F modifier) {
  modifier(base);
  return base;
}
