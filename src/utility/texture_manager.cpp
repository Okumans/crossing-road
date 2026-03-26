#include "texture_manager.hpp"
#include <cstdint>
#include <glad/gl.h>
#include <memory>
#include <unordered_map>

namespace std {
template <> struct hash<TextureName> {
  std::size_t operator()(const TextureName &k) const noexcept {
    return std::hash<std::string>{}(k.name);
  }
};
} // namespace std

std::unordered_map<TextureName, std::shared_ptr<Texture>>
    TextureManager::textures;

std::shared_ptr<Texture> TextureManager::loadTexture(TextureName name,
                                                     TextureType type,
                                                     const char *texture_path,
                                                     bool flip_vertical) {
  textures[name] = std::make_shared<Texture>(texture_path, type, flip_vertical);
  return TextureManager::textures.at(name);
}

Texture TextureManager::loadTexture(TextureType type, const char *texture_path,
                                    bool flip_vertical) {
  return Texture(texture_path, type, flip_vertical);
}

std::shared_ptr<Texture> TextureManager::manage(TextureName name,
                                                Texture &&texture) {
  textures[name] = std::make_shared<Texture>(std::move(texture));
  return TextureManager::textures.at(name);
}

Texture TextureManager::generateStaticWhiteTexture() {
  GLuint white_texture;

  glCreateTextures(GL_TEXTURE_2D, 1, &white_texture);

  // 1 mip level, RGBA8 format, 1x1 size
  glTextureStorage2D(white_texture, 1, GL_RGBA8, 1, 1);

  const uint8_t white[] = {0xFF, 0xFF, 0xFF, 0xFF};
  glTextureSubImage2D(white_texture, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                      white);

  glTextureParameteri(white_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(white_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTextureParameteri(white_texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTextureParameteri(white_texture, GL_TEXTURE_WRAP_T, GL_REPEAT);

  return Texture(white_texture, TextureType::DIFFUSE, false);
}

Texture &TextureManager::getTextureRef(TextureName name) {
  return *TextureManager::textures.at(name);
}

std::shared_ptr<Texture> TextureManager::getTexture(TextureName name) {
  return TextureManager::textures.at(name);
}

bool TextureManager::exists(TextureName name) {
  return TextureManager::textures.find(name) != TextureManager::textures.end();
}
