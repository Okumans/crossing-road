#include "model.hpp"
#include "graphics/material.hpp"
#include "graphics/texture.hpp"
#include "resource/texture_manager.hpp"

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cassert>
#include <glm/fwd.hpp>

#include <format>
#include <generator>
#include <memory>
#include <print>
#include <string_view>

Model::Model(const char *path, bool flip_vertical) : m_path(path) {
  _loadModel(path, flip_vertical);
}

Model::Model(Model &&other) noexcept
    : m_meshes(std::move(other.m_meshes)),
      m_directory(std::move(other.m_directory)),
      m_path(std::move(other.m_path)) {
  other.m_meshes.clear();
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
  auto last_slash = path_sv.find_last_of('/');
  if (last_slash != std::string_view::npos) {
    m_directory = std::string(path_sv.substr(0, last_slash));
  } else {
    m_directory = ".";
  }

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

    glm::vec3 tangent = (mesh->mTangents) ? glm::vec3(mesh->mTangents[i].x,
                                                      mesh->mTangents[i].y,
                                                      mesh->mTangents[i].z)
                                          : glm::vec3(1.0f, 0.0f, 0.0f);

    glm::vec3 bitangent =
        (mesh->mBitangents)
            ? glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y,
                        mesh->mBitangents[i].z)
            : glm::vec3(0.0f, 1.0f, 0.0f);

    vertices.emplace_back(position, normal, texCoords, tangent, bitangent);
  }

  for (size_t i = 0; i < mesh->mNumFaces; ++i) {
    aiFace face = mesh->mFaces[i];

    for (size_t j = 0; j < face.mNumIndices; ++j) {
      indices.push_back(face.mIndices[j]);
    }
  }

  // Get material colors
  aiColor4D diffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
  if (mesh->mMaterialIndex >= 0) {
    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
    if (AI_SUCCESS !=
        aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuseColor)) {
      // fallback if failed
    }
  }
  glm::vec3 baseColor(diffuseColor.r, diffuseColor.g, diffuseColor.b);

  MaterialBuilder mat_builder = Material::builder();

  if (mesh->mMaterialIndex >= 0) {
    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

    // Diffuse / Base Color
    auto diffuseTextures =
        _loadMaterialTextures(material, scene, aiTextureType_DIFFUSE,
                              TextureType::DIFFUSE, flip_vertical);
    auto baseColorTextures =
        _loadMaterialTextures(material, scene, aiTextureType_BASE_COLOR,
                              TextureType::DIFFUSE, flip_vertical);

    bool foundDiffuse = false;
    for (auto &&tex : diffuseTextures) {
      mat_builder.setDiffuse(tex);
      foundDiffuse = true;
      break;
    }
    if (!foundDiffuse) {
      for (auto &&tex : baseColorTextures) {
        mat_builder.setDiffuse(tex);
        break;
      }
    }

    // Normal / Height (OBJ often uses Height for Normals)
    auto normalTextures =
        _loadMaterialTextures(material, scene, aiTextureType_NORMALS,
                              TextureType::NORMAL, flip_vertical);
    auto heightTextures =
        _loadMaterialTextures(material, scene, aiTextureType_HEIGHT,
                              TextureType::NORMAL, flip_vertical);

    bool foundNormal = false;
    for (auto &&tex : normalTextures) {
      mat_builder.setNormal(tex);
      foundNormal = true;
      break;
    }
    if (!foundNormal) {
      // Only use height as normal if it's actually intended as such (common in
      // OBJ)
      for (auto &&tex : heightTextures) {
        mat_builder.setNormal(tex);
        break;
      }
    }

    // Metallic
    for (std::shared_ptr<Texture> &&texture :
         _loadMaterialTextures(material, scene, aiTextureType_METALNESS,
                               TextureType::METALLIC, flip_vertical)) {
      mat_builder.setMetallic(texture);
    }

    // Roughness
    for (std::shared_ptr<Texture> &&texture :
         _loadMaterialTextures(material, scene, aiTextureType_DIFFUSE_ROUGHNESS,
                               TextureType::ROUGHNESS, flip_vertical)) {
      mat_builder.setRoughness(texture);
    }

    // Ambient Occlusion
    for (std::shared_ptr<Texture> &&texture :
         _loadMaterialTextures(material, scene, aiTextureType_AMBIENT_OCCLUSION,
                               TextureType::AO, flip_vertical)) {
      mat_builder.setAO(texture);
    }
  }

  Material material = mat_builder.create();

  return Mesh(std::move(vertices), std::move(indices), std::move(material),
              baseColor);
}

std::generator<std::shared_ptr<Texture>>
Model::_loadMaterialTextures(aiMaterial *mat, const aiScene *scene,
                             aiTextureType type, TextureType typeName,
                             bool flip_vertical) {
  for (size_t i = 0; i < mat->GetTextureCount(type); ++i) {
    aiString path;
    mat->GetTexture(type, i, &path);

    // Create a unique texture name to avoid collisions across different models
    std::string unique_name;
    if (path.C_Str()[0] == '*') {
      unique_name = std::format("{}:{}", m_path, path.C_Str());
    } else {
      unique_name = std::format("{}/{}", m_directory, path.C_Str());
    }
    TextureName texture_name(std::move(unique_name));

    if (!TextureManager::exists(texture_name)) {
      std::shared_ptr<Texture> texture;
      // Handle embedded textures
      if (path.C_Str()[0] == '*') {
        int index = std::stoi(path.C_Str() + 1);
        aiTexture *aiTex = scene->mTextures[index];

        if (aiTex->mHeight == 0) {
          // Compressed texture (png, jpg, etc.)
          texture =
              TextureManager::loadTexture(texture_name, typeName, aiTex->pcData,
                                          aiTex->mWidth, flip_vertical);
        } else {
          // Uncompressed texture (RGBA8888)
          texture = TextureManager::loadTexture(
              texture_name, typeName, aiTex->pcData,
              aiTex->mWidth * aiTex->mHeight * 4, flip_vertical);
        }
      } else {
        texture = TextureManager::loadTexture(
            texture_name, typeName, unique_name.c_str(), flip_vertical);
      }

      co_yield texture;
    } else {
      std::shared_ptr<Texture> texture =
          TextureManager::getTexture(texture_name);

      co_yield texture;
    }
  }
}
