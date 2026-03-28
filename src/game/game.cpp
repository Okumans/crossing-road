#include "game.hpp"
#include "glm/trigonometric.hpp"
#include "graphics/material.hpp"
#include "resource/lighting_manager.hpp"
#include "resource/material_manager.hpp"
#include "resource/model_manager.hpp"
#include "resource/shader_manager.hpp"

#include "glad/gl.h"
#include "glm/fwd.hpp"
#include "resource/texture_manager.hpp"
#include "scene/object.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

Game::Game() : m_player(nullptr), m_skybox(std::make_unique<Skybox>()) {}

Game::~Game() {
  glDeleteFramebuffers(1, &m_shadowMapFBO);
  glDeleteTextures(1, &m_shadowMapTex);
}

void Game::setup() {
  // Setup shadow map FBO
  glGenFramebuffers(1, &m_shadowMapFBO);
  glGenTextures(1, &m_shadowMapTex);
  glBindTexture(GL_TEXTURE_2D, m_shadowMapTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH,
               SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float borderColor[] = {1.0, 1.0, 1.0, 1.0};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

  glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         m_shadowMapTex, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Initialize static textures
  if (!TextureManager::exists(STATIC_WHITE_TEXTURE))
    TextureManager::manage(STATIC_WHITE_TEXTURE,
                           TextureManager::generateStaticWhiteTexture());
  if (!TextureManager::exists(STATIC_BLACK_TEXTURE))
    TextureManager::manage(STATIC_BLACK_TEXTURE,
                           TextureManager::generateStaticBlackTexture());
  if (!TextureManager::exists(STATIC_NORMAL_TEXTURE))
    TextureManager::manage(STATIC_NORMAL_TEXTURE,
                           TextureManager::generateStaticNormalTexture());

  // Load Skybox
  TextureManager::loadCubemap(
      TextureName("skybox"),
      {
          ASSETS_PATH "/textures/skybox/px.png", // Right
          ASSETS_PATH "/textures/skybox/nx.png", // Left
          ASSETS_PATH "/textures/skybox/py.png", // Top
          ASSETS_PATH "/textures/skybox/ny.png", // Bottom
          ASSETS_PATH "/textures/skybox/pz.png", // Front
          ASSETS_PATH "/textures/skybox/nz.png"  // Back
      });

  const Material &grass_mat_1 = MaterialManager::getMaterial("grass_1");
  const Material &grass_mat_2 = MaterialManager::getMaterial("grass_2");
  const Material &road_mat_1 = MaterialManager::getMaterial("road_1");
  const Material &road_mat_2 = MaterialManager::getMaterial("road_2");

  auto waterTex = TextureManager::getTexture(TextureName("water"));
  const Material water_mat = Material::builder().setDiffuse(waterTex).create();

  m_map.addRow(RowType::GRASS, grass_mat_1);
  m_map.addRow(RowType::GRASS, grass_mat_2);

  m_map.addRow(RowType::ROAD, road_mat_1, 0.04f);
  m_map.addRow(RowType::ROAD, road_mat_2, 0.04f);

  m_map.addRow(RowType::GRASS, grass_mat_1);

  m_map.addRow(RowType::WATER, water_mat, -0.2f, water_mat);

  m_map.addRow(RowType::GRASS, grass_mat_1);
  m_map.addRow(RowType::GRASS, grass_mat_2);

  m_player =
      std::make_unique<Object>(ModelManager::getModel(ModelName::CHICKEN));
  m_player->setRotation({0, glm::radians(-90.0f), 0});
  m_player->setPosition({0.25f, 0.0f, 0.25f});

  // Setup Lights
  LightingManager::clearLights();

  // 1. "Sun" Light (Directional, casts shadows)
  LightingManager::addLight({.type = LightType::DIRECTIONAL,
                             .position = glm::vec3(-0.5f, -1.0f, -0.3f),
                             .color = glm::vec3(5.0f, 4.8f, 4.2f),
                             .castsShadows = true});

  // 2. Sky Blue Fill Light (Directional)
  LightingManager::addLight({.type = LightType::DIRECTIONAL,
                             .position = glm::vec3(0.5f, -1.0f, 0.2f),
                             .color = glm::vec3(0.5f, 0.7f, 1.0f)});

  // 3. Ground Bounce Fill (Point light)
  LightingManager::addLight({.type = LightType::POINT,
                             .position = glm::vec3(0.0f, -5.0f, 0.0f),
                             .color = glm::vec3(1.0f, 0.8f, 0.6f)});
}

void Game::update(double delta_time) { ; }

void Game::render(double delta_time, Camera &camera) {
  glEnable(GL_DEPTH_TEST);

  // 1. Shadow Pass
  m_lightSpaceMatrix =
      LightingManager::calculateLightSpaceMatrix(m_player->getPosition());

  Shader &shadow_shader = ShaderManager::getShader(ShaderType::SHADOW);
  shadow_shader.use();
  shadow_shader.setMat4("u_LightSpaceMatrix", m_lightSpaceMatrix);

  glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
  glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFBO);
  glClear(GL_DEPTH_BUFFER_BIT);
  glCullFace(GL_FRONT);

  m_map.draw(shadow_shader);
  m_player->draw(shadow_shader);

  glCullFace(GL_BACK);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // 2. Main Pass
  glViewport(0, 0, (int)camera.getSceneWidth(), (int)camera.getSceneHeight());
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::mat4 projection = camera.getProjectionMatrix();
  glm::mat4 view = camera.getViewMatrix();

  Shader &pbr_shader = ShaderManager::getShader(ShaderType::PBR);
  pbr_shader.use();
  pbr_shader.setMat4("u_Projection", projection);
  pbr_shader.setMat4("u_View", view);
  pbr_shader.setVec3("u_CameraPos", camera.Position);
  pbr_shader.setMat4("u_LightSpaceMatrix", m_lightSpaceMatrix);

  // Bind Skybox
  auto skyboxTex = TextureManager::getTexture(TextureName("skybox"));
  glBindTextureUnit(10, skyboxTex->getTexID());
  pbr_shader.setInt("u_Skybox", 10);

  // Bind Shadow Map
  glBindTextureUnit(11, m_shadowMapTex);
  pbr_shader.setInt("u_ShadowMap", 11);

  // Lighting setup
  LightingManager::apply(pbr_shader);

  // Default PBR factors for things without textures
  pbr_shader.setFloat("u_MetallicFactor", 1.0f);
  pbr_shader.setFloat("u_RoughnessFactor", 1.0f);
  pbr_shader.setFloat("u_HeightScale", 0.05f);

  // Draw Map
  m_map.draw(pbr_shader);

  Object tree_1 = Object(ModelManager::getModel(ModelName::TREE_1));
  tree_1.setPosition({0.25f, 0.0f, 0.25f});
  tree_1.setScale(0.045f);
  tree_1.draw(pbr_shader);

  m_player->draw(pbr_shader);

  // Draw Skybox Mesh
  Shader &skybox_shader = ShaderManager::getShader(ShaderType::SKYBOX);
  m_skybox->draw(camera, skybox_shader, *skyboxTex);

  glDisable(GL_DEPTH_TEST);
}

void Game::moveForward() {
  if (m_player) {
    glm::vec3 pos = m_player->getPosition();
    pos.z -= 0.5f;
    m_player->setPosition(pos);
  }
}

void Game::moveLeft(double delta_time) {
  if (m_player) {
    glm::vec3 pos = m_player->getPosition();
    pos.x -= 2.0f * (float)delta_time;
    m_player->setPosition(pos);
  }
}

void Game::moveRight(double delta_time) {
  if (m_player) {
    glm::vec3 pos = m_player->getPosition();
    pos.x += 2.0f * (float)delta_time;
    m_player->setPosition(pos);
  }
}
