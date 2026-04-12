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

// template <typename Key, typename Value,
//           size_t Size = magic_enum::enum_count<Key>()>
// class EnumMap {
// private:
//   std::array<Value, Size> _data;
//
//   // Helper to convert enum to index
//   constexpr size_t to_idx(Key key) const { return static_cast<size_t>(key); }
//
// public:
//   // Default constructor
//   EnumMap() = default;
//
//   struct Entry {
//     Key key;
//     Value value;
//   };
//
//   // // Initializer list constructor
//   // constexpr EnumMap(std::initializer_list<std::pair<Key, Value>> list) {
//   //   for (auto &item :
//   //        const_cast<std::initializer_list<std::pair<Key, Value>> &>(list))
//   {
//   //     // We use const_cast because initializer_list elements are const,
//   //     // and we need to move the Value (especially for unique_ptr)
//   //     _data[to_idx(item.first)] = std::move(item.second);
//   //   }
//   // }
//   // 1. Variadic constructor for: EnumMap(Entry{k, v}, Entry{k, v})
//   template <typename... Entries>
//   constexpr EnumMap(Entries &&...entries) : _data{} {
//     ((_data[to_idx(entries.key)] = std::move(entries.value)), ...);
//   }
//
//   // 2. Array-based constructor for: EnumMap({ {k, v}, {k, v} })
//   // This allows the {{...}, {...}} syntax to work
//   template <size_t N> constexpr EnumMap(Entry (&&entries)[N]) : _data{} {
//     for (size_t i = 0; i < N; ++i) {
//       _data[to_idx(entries[i].key)] = std::move(entries[i].value);
//     }
//   }
//
//   // Access with bounds checking
//   Value &at(Key key) { return _data.at(to_idx(key)); }
//
//   const Value &at(Key key) const { return _data.at(to_idx(key)); }
//
//   // Standard operator[] (no bounds checking, like std::array)
//   Value &operator[](Key key) { return _data[to_idx(key)]; }
//
//   const Value &operator[](Key key) const { return _data[to_idx(key)]; }
//
//   size_t size() const { return Size; }
//
//   // Allows: for (auto& x : map)
//   auto begin() noexcept { return _data.begin(); }
//   auto end() noexcept { return _data.end(); }
//
//   // Allows: for (const auto& x : map)
//   auto begin() const noexcept { return _data.begin(); }
//   auto end() const noexcept { return _data.end(); }
//
//   // Explicit const iterators
//   auto cbegin() const noexcept { return _data.cbegin(); }
//   auto cend() const noexcept { return _data.cend(); }
//
//   auto pairs() {
//     return std::views::iota(size_t(0), Size) |
//            std::views::transform([this](size_t i) {
//              return std::pair<Key, Value &>(static_cast<Key>(i), _data[i]);
//            });
//   }
//
//   auto pairs() const {
//     return std::views::iota(size_t(0), Size) |
//            std::views::transform([this](size_t i) {
//              return std::pair<Key, const Value &>(static_cast<Key>(i),
//                                                   _data[i]);
//            });
//   }
// };
