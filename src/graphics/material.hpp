#pragma once

#include "graphics/texture.hpp"
#include "resource/texture_manager.hpp"

#include <cassert>
#include <memory>

class MaterialBuilder;

class Material {
private:
  std::shared_ptr<Texture> m_diffuse;
  std::shared_ptr<Texture> m_normal;
  std::shared_ptr<Texture> m_height;
  std::shared_ptr<Texture> m_metallic;
  std::shared_ptr<Texture> m_roughness;
  std::shared_ptr<Texture> m_ao;

public:
  Material(std::shared_ptr<Texture> diffuse, std::shared_ptr<Texture> normal,
           std::shared_ptr<Texture> height, std::shared_ptr<Texture> metallic,
           std::shared_ptr<Texture> roughness, std::shared_ptr<Texture> ao)
      : m_diffuse(std::move(diffuse)), m_normal(std::move(normal)),
        m_height(std::move(height)), m_metallic(std::move(metallic)),
        m_roughness(std::move(roughness)), m_ao(std::move(ao)) {}

  Material(Material &&other) noexcept
      : m_diffuse(std::move(other.m_diffuse)),
        m_normal(std::move(other.m_normal)),
        m_height(std::move(other.m_height)),
        m_metallic(std::move(other.m_metallic)),
        m_roughness(std::move(other.m_roughness)), m_ao(std::move(other.m_ao)) {
  }

  Material(const Material &other) = default;

  // Getters returning const references for zero-overhead access
  const std::shared_ptr<Texture> &getDiffuse() const { return m_diffuse; }
  const std::shared_ptr<Texture> &getNormal() const { return m_normal; }
  const std::shared_ptr<Texture> &getHeight() const { return m_height; }
  const std::shared_ptr<Texture> &getMetallic() const { return m_metallic; }
  const std::shared_ptr<Texture> &getRoughness() const { return m_roughness; }
  const std::shared_ptr<Texture> &getAO() const { return m_ao; }

  static MaterialBuilder builder();
};

class MaterialBuilder {
private:
  std::shared_ptr<Texture> m_diffuse = nullptr;
  std::shared_ptr<Texture> m_normal = nullptr;
  std::shared_ptr<Texture> m_height = nullptr;
  std::shared_ptr<Texture> m_metallic = nullptr;
  std::shared_ptr<Texture> m_roughness = nullptr;
  std::shared_ptr<Texture> m_ao = nullptr;

public:
  MaterialBuilder()
      : m_diffuse(nullptr), m_normal(nullptr), m_height(nullptr),
        m_metallic(nullptr), m_roughness(nullptr), m_ao(nullptr) {}

  MaterialBuilder &setDiffuse(std::shared_ptr<Texture> diffuse) {
    m_diffuse = std::move(diffuse);
    return *this;
  }

  MaterialBuilder &setNormal(std::shared_ptr<Texture> normal) {
    m_normal = std::move(normal);
    return *this;
  }

  MaterialBuilder &setHeight(std::shared_ptr<Texture> height) {
    m_height = std::move(height);
    return *this;
  }

  MaterialBuilder &setMetallic(std::shared_ptr<Texture> metallic) {
    m_metallic = std::move(metallic);
    return *this;
  }

  MaterialBuilder &setRoughness(std::shared_ptr<Texture> roughness) {
    m_roughness = std::move(roughness);
    return *this;
  }

  MaterialBuilder &setAO(std::shared_ptr<Texture> ao) {
    m_ao = std::move(ao);
    return *this;
  }

  Material create() {
    assert(TextureManager::exists(STATIC_WHITE_TEXTURE));
    assert(TextureManager::exists(STATIC_BLACK_TEXTURE));
    assert(TextureManager::exists(STATIC_NORMAL_TEXTURE));

    // Diffuse fallback
    if (m_diffuse == nullptr)
      m_diffuse = TextureManager::getTexture(STATIC_WHITE_TEXTURE);

    // Metallic fallback
    if (m_metallic == nullptr)
      m_metallic = TextureManager::getTexture(STATIC_BLACK_TEXTURE);

    // Roughness fallback
    if (m_roughness == nullptr)
      m_roughness = TextureManager::getTexture(STATIC_WHITE_TEXTURE);

    // Ambient Occlusion fallback
    if (m_ao == nullptr)
      m_ao = TextureManager::getTexture(STATIC_WHITE_TEXTURE);

    // Normal fallback
    if (m_normal == nullptr)
      m_normal = TextureManager::getTexture(STATIC_NORMAL_TEXTURE);

    // Height fallback
    if (m_height == nullptr)
      m_height = TextureManager::getTexture(STATIC_BLACK_TEXTURE);

    return Material(std::move(m_diffuse), std::move(m_normal),
                    std::move(m_height), std::move(m_metallic),
                    std::move(m_roughness), std::move(m_ao));
  }
};

inline MaterialBuilder Material::builder() { return MaterialBuilder(); }
