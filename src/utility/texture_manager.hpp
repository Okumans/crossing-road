#pragma once

#include <glad/gl.h>
#include <memory>
#include <unordered_map>

struct TextureName {
  const std::string name;

  constexpr TextureName(const char *name) : name(name) {}
  constexpr TextureName(std::string &&name) : name(name) {}

  bool operator==(const TextureName &) const = default;
};

const TextureName STATIC_WHITE_TEXTURE = TextureName("STATIC_WHITE_TEXTURE");

enum class TextureType : uint8_t { DIFFUSE, SPECULAR };

class Texture {
private:
  bool m_ownTex;
  TextureType m_type;
  GLuint m_texID;

public:
  Texture(const char *path, TextureType type, bool flip_vertical = false);
  Texture(GLuint tex_id, TextureType type, bool own = false);
  ~Texture();

  Texture(const Texture &) = delete;
  Texture &operator=(const Texture &) = delete;

  Texture(Texture &&other) noexcept;

  GLuint getTexID() const { return m_texID; };
  TextureType getType() const { return m_type; };

private:
  GLuint _loadTexture(const char *path, bool flip_vertical);
};

class TextureManager {
public:
  static std::unordered_map<TextureName, std::shared_ptr<Texture>> textures;

  static std::shared_ptr<Texture> loadTexture(TextureName name,
                                              TextureType type,
                                              const char *texture_path,
                                              bool flip_vertical = false);

  static Texture loadTexture(TextureType type, const char *texture_path,
                             bool flip_vertical = false);

  static std::shared_ptr<Texture> manage(TextureName name, Texture &&texture);

  static std::shared_ptr<Texture> getTexture(TextureName name);

  static Texture &getTextureRef(TextureName name);

  static Texture generateStaticWhiteTexture();

  static bool exists(TextureName name);
};
