#include "model.hpp"
#include "assimp/material.h"
#include "glm/fwd.hpp"
#include "utility/mesh.hpp"
#include "utility/texture_manager.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <generator>
#include <memory>
#include <print>
#include <string_view>

Model::Model(const char *path, bool flip_vertical) {
  _loadModel(path, flip_vertical);
}

void Model::draw(Shader &shader) {
  for (Mesh &mesh : m_meshes) {
    mesh.draw(shader);
  }
}

void Model::_loadModel(const char *path, bool flip_vertical) {
  Assimp::Importer import;
  const aiScene *scene = import.ReadFile(
      path, aiProcess_Triangulate | aiProcess_FlipUVs |
                aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices |
                aiProcess_PreTransformVertices);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    std::println("ERROR::ASSIMP::{}", import.GetErrorString());

    return;
  }

  std::string_view path_sv = std::string_view(path);
  m_directory = path_sv.substr(0, path_sv.find_last_of('/'));

  _processNode(scene->mRootNode, scene, flip_vertical);
}

void Model::_processNode(aiNode *node, const aiScene *scene,
                         bool flip_vertical) {
  for (size_t i = 0; i < node->mNumMeshes; ++i) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    m_meshes.push_back(_processMesh(mesh, scene, flip_vertical));
  }

  for (size_t i = 0; i < node->mNumChildren; ++i) {
    _processNode(node->mChildren[i], scene, flip_vertical);
  }
}

Mesh Model::_processMesh(aiMesh *mesh, const aiScene *scene,
                         bool flip_vertical) {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  std::vector<std::shared_ptr<Texture>> textures;

  // Color fallback if texture don't exists
  aiColor3D baseColor(1.0f, 1.0f, 1.0f);

  vertices.reserve(mesh->mNumVertices);
  indices.reserve(mesh->mNumFaces);

  for (size_t i = 0; i < mesh->mNumVertices; ++i) {
    glm::vec3 position(mesh->mVertices[i].x, mesh->mVertices[i].y,
                       mesh->mVertices[i].z);

    glm::vec3 normal = (mesh->mNormals)
                           ? glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y,
                                       mesh->mNormals[i].z)
                           : glm::vec3(0.0f);

    glm::vec2 texCoords = (mesh->mTextureCoords[0])
                              ? glm::vec2(mesh->mTextureCoords[0][i].x,
                                          mesh->mTextureCoords[0][i].y)
                              : glm::vec2(0.0f);

    vertices.emplace_back(position, normal, texCoords);
  }

  for (size_t i = 0; i < mesh->mNumFaces; ++i) {
    aiFace face = mesh->mFaces[i];

    for (size_t j = 0; j < face.mNumIndices; ++j) {
      indices.push_back(face.mIndices[j]);
    }
  }

  if (mesh->mMaterialIndex >= 0) {
    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

    // No diffuse texture, fall back to white static texture
    if (material->GetTextureCount(aiTextureType_DIFFUSE) <= 0) {
      if (!TextureManager::exists(STATIC_WHITE_TEXTURE))
        textures.push_back(TextureManager::manage(
            STATIC_WHITE_TEXTURE,
            TextureManager::generateStaticWhiteTexture()));
      else
        textures.push_back(TextureManager::getTexture(STATIC_WHITE_TEXTURE));

      material->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor);
    }

    for (std::shared_ptr<Texture> &&texture :
         _loadMaterialTextures(material, aiTextureType_DIFFUSE,
                               TextureType::DIFFUSE, flip_vertical)) {
      textures.push_back(texture);
    }

    for (std::shared_ptr<Texture> &&texture :
         _loadMaterialTextures(material, aiTextureType_SPECULAR,
                               TextureType::SPECULAR, flip_vertical)) {
      textures.push_back(texture);
    }
  }

  return Mesh(vertices, indices, textures,
              glm::vec3(baseColor.r, baseColor.g, baseColor.b));
}

std::generator<std::shared_ptr<Texture>>
Model::_loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                             TextureType typeName, bool flip_vertical) {
  std::vector<std::shared_ptr<Texture>> textures;

  for (size_t i = 0; i < mat->GetTextureCount(type); ++i) {
    aiString path;
    mat->GetTexture(type, i, &path);

    TextureName texture_name(path.C_Str());

    if (!TextureManager::exists(texture_name)) {
      std::string texture_path =
          std::format("{}/{}", m_directory, path.C_Str());

      std::shared_ptr<Texture> texture = TextureManager::loadTexture(
          texture_name, typeName, texture_path.data(), flip_vertical);

      co_yield texture;
    } else {
      std::shared_ptr<Texture> texture =
          TextureManager::getTexture(texture_name);

      co_yield texture;
    }
  }
}
