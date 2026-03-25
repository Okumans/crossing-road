#pragma once

#include "shader.h"
#include "utility/mesh.hpp"
#include "utility/texture_manager.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <generator>

class Model {
private:
  // model data
  std::vector<Mesh> m_meshes;
  std::string_view m_directory;

public:
  Model(const char *path, bool flip_vertical = false);

  Model(const Model &) = delete;
  Model &operator=(Model &) const = delete;

  Model(Model &&) noexcept;

  void draw(Shader &shader);

private:
  void _loadModel(const char *path, bool flip_vertical);
  void _processNode(aiNode *node, const aiScene *scene, bool flip_vertical);
  Mesh _processMesh(aiMesh *mesh, const aiScene *scene, bool flip_vertical);
  std::generator<std::shared_ptr<Texture>>
  _loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                        TextureType typeName, bool flip_vertical);
};
