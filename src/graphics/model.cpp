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

#include <generator>
#include <memory>
#include <print>
#include <string_view>

Model::Model(const char *path, bool flip_vertical) {
  _loadModel(path, flip_vertical);
}

Model::Model(Model &&other) noexcept
    : m_meshes(std::move(other.m_meshes)), m_directory(other.m_directory) {
  other.m_meshes.clear();
  m_directory = "";
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

  MaterialBuilder mat_builder = Material::builder();

  if (mesh->mMaterialIndex >= 0) {
    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

    // Diffuse
    for (std::shared_ptr<Texture> &&texture :
         _loadMaterialTextures(material, scene, aiTextureType_DIFFUSE,
                               TextureType::DIFFUSE, flip_vertical)) {
      mat_builder.setDiffuse(texture);
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

    // Normal
    for (std::shared_ptr<Texture> &&texture :
         _loadMaterialTextures(material, scene, aiTextureType_NORMALS,
                               TextureType::NORMAL, flip_vertical)) {
      mat_builder.setNormal(texture);
    }

    // Height
    for (std::shared_ptr<Texture> &&texture :
         _loadMaterialTextures(material, scene, aiTextureType_HEIGHT,
                               TextureType::HEIGHT, flip_vertical)) {
      mat_builder.setHeight(texture);
    }
  }

  Material material = mat_builder.create();

  return Mesh(std::move(vertices), std::move(indices), std::move(material),
              glm::vec3(baseColor.r, baseColor.g, baseColor.b));
}

std::generator<std::shared_ptr<Texture>>
Model::_loadMaterialTextures(aiMaterial *mat, const aiScene *scene,
                             aiTextureType type, TextureType typeName,
                             bool flip_vertical) {
  for (size_t i = 0; i < mat->GetTextureCount(type); ++i) {
    aiString path;
    mat->GetTexture(type, i, &path);

    TextureName texture_name(path.C_Str());

    if (!TextureManager::exists(texture_name)) {
      std::shared_ptr<Texture> texture;
      // Handle embedded textures
      if (path.C_Str()[0] == '*') {
        int index = std::stoi(path.C_Str() + 1);
        aiTexture *aiTex = scene->mTextures[index];

        if (aiTex->mHeight == 0) {
          // Compressed texture (png, jpg, etc.)
          texture = TextureManager::loadTexture(
              texture_name, typeName, aiTex->pcData, aiTex->mWidth,
              flip_vertical);
        } else {
          // Uncompressed texture (RGBA8888)
          // We need to handle this case too, but most often it's compressed in
          // GLB/FBX. For uncompressed, we'd need a separate loader or use
          // stbi_load_from_memory with raw data.
          texture = TextureManager::loadTexture(
              texture_name, typeName, aiTex->pcData,
              aiTex->mWidth * aiTex->mHeight * 4, flip_vertical);
        }
      } else {
        std::string texture_path =
            std::format("{}/{}", m_directory, path.C_Str());

        texture = TextureManager::loadTexture(texture_name, typeName,
                                              texture_path.data(), flip_vertical);
      }

      co_yield texture;
    } else {
      std::shared_ptr<Texture> texture =
          TextureManager::getTexture(texture_name);

      co_yield texture;
    }
  }
}
