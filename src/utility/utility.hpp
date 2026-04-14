#pragma once

#include "graphics/material.hpp"
#include "graphics/texture.hpp"
#include "resource/material_manager.hpp"
#include "resource/texture_manager.hpp"

#include <array>
#include <concepts>
#include <filesystem>
#include <format>
#include <memory>
#include <random>
#include <ranges>
#include <stdexcept>
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
  template <std::integral T = int, std::ranges::input_range R>
  static T randWeighted(T min, T max, const R &weights) {
    // weights[0] corresponds to min, weights[1] to min + 1, etc.
    std::discrete_distribution<T> dist(std::begin(weights), std::end(weights));
    return min + dist(s_engine);
  }

  template <std::integral T = int>
  static T randWeighted(T min, T max, std::initializer_list<double> weights) {
    return randWeighted<T, std::initializer_list<double>>(min, max, weights);
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

template <typename T> class NotInitialized {
private:
  std::optional<T> storage;

public:
  // Throws if accessed before init() is called
  T &ensureInitialized() {
    if (!storage.has_value()) {
      throw std::runtime_error("Attempted to access unitialized object.");
    }
    return *storage;
  }

  // Const version for read-only access
  const T &ensureInitialized() const {
    if (!storage.has_value()) {
      throw std::runtime_error("Attempted to access unitialized object.");
    }
    return *storage;
  }

  bool isInitialized() const { return storage.has_value(); }

  // Variadic template to construct T in-place
  template <typename... Args> void init(Args &&...args) {
    if (storage.has_value()) {
      throw std::runtime_error("Object is already initialized.");
    }
    storage.emplace(std::forward<Args>(args)...);
  }
};

template <typename T> struct DefaultValidator {
  constexpr bool operator()(const T &) const { return true; }
};

template <typename T, typename Validator = DefaultValidator<T>>
class SettableNotInitialized {
private:
  std::optional<T> storage;

public:
  template <typename Key, typename Value>
    requires requires(T t, Key k, Value v) { t[k] = std::forward<Value>(v); }
  constexpr bool set(Key &&key, Value &&value) {
    if (!storage.has_value()) {
      storage.emplace();
    }

    (*storage)[std::forward<Key>(key)] = std::forward<Value>(value);

    return Validator{}(*storage);
  }

  template <typename Key>
    requires requires(T t, Key k) { t[k]; }
  constexpr decltype(auto) getUnvalidated(Key &&key) {
    if (storage.has_value()) {
      return (*storage)[std::forward<Key>(key)];
    }

    throw std::runtime_error("Attempted to access elements before underlying "
                             "storage was allocated.");
  }

  template <typename Key>
    requires requires(const T t, Key k) { t[k]; }
  constexpr decltype(auto) getUnvalidated(Key &&key) const {
    if (storage.has_value()) {
      return (*storage)[std::forward<Key>(key)];
    }

    throw std::runtime_error("Attempted to access elements before underlying "
                             "storage was allocated.");
  }

  constexpr T &ensureInitialized() {
    if (!isInitialized()) {
      throw std::runtime_error(
          "Attempted to access uninitialized or incomplete object.");
    }
    return *storage;
  }

  constexpr const T &ensureInitialized() const {
    if (!isInitialized()) {
      throw std::runtime_error(
          "Attempted to access uninitialized or incomplete object.");
    }
    return *storage;
  }

  constexpr bool isInitialized() const {
    return storage.has_value() && Validator{}(*storage);
  }

  constexpr void clear() { storage.reset(); }
};
