#include "game.hpp"
#include "game/row.hpp"
#include "glm/trigonometric.hpp"
#include "graphics/material.hpp"
#include "resource/lighting_manager.hpp"
#include "resource/material_manager.hpp"
#include "resource/model_manager.hpp"
#include "resource/shader_manager.hpp"

#include "glad/gl.h"
#include "glm/fwd.hpp"
#include "resource/texture_manager.hpp"
#include "scene/car.hpp"
#include "scene/object.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <print>

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
  if (!TextureManager::exists(STATIC_PBR_DEFAULT_TEXTURE))
    TextureManager::manage(STATIC_PBR_DEFAULT_TEXTURE,
                           TextureManager::generateStaticPBRDefaultTexture());

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

  // Start with some grass
  m_map.addRow(RowType::GRASS, grass_mat_1, 0.0f);
  m_map.addRow(RowType::GRASS, grass_mat_2, 0.0f);
  m_map.addRow(RowType::GRASS, grass_mat_1, 0.0f);

  // A road
  m_map.addRow(RowType::ROAD, road_mat_1, 0.05f);
  m_map.addRow(RowType::ROAD, road_mat_2, 0.05f);
  m_map.addRow(RowType::ROAD, road_mat_1, 0.05f);

  // More grass
  m_map.addRow(RowType::GRASS, grass_mat_2, 0.0f);

  // A river (recessed)
  m_map.addRow(RowType::GRASS, grass_mat_1, 0.0f);
  m_map.addRow(RowType::WATER, water_mat, -0.2f, grass_mat_1);
  m_map.addRow(RowType::WATER, water_mat, -0.2f, grass_mat_1);
  m_map.addRow(RowType::GRASS, grass_mat_1, 0.0f);

  // A raised grass area
  m_map.addRow(RowType::GRASS, grass_mat_2, 0.1f);
  m_map.addRow(RowType::GRASS, grass_mat_1, 0.1f);
  m_map.addRow(RowType::GRASS, grass_mat_2, 0.0f);

  // Another road
  m_map.addRow(RowType::ROAD, road_mat_2, 0.05f);
  m_map.addRow(RowType::ROAD, road_mat_1, 0.05f);

  // End with grass
  m_map.addRow(RowType::GRASS, grass_mat_1, 0.0f);
  m_map.addRow(RowType::GRASS, grass_mat_2, 0.0f);

  // Helper to add random greenery to a grass row (very sparse)
  auto populateGreenery = [&](Row &row) {
    for (int i = -10; i <= 10; ++i) {
      if (std::abs(i) <= 1)
        continue; // Keep center clear for player

      float chance = (float)rand() / RAND_MAX;
      glm::vec3 custom_position_offset = glm::vec3(0.0f);

      if (chance > 0.95f) { // ~5% chance
        float subChance = (float)rand() / RAND_MAX;
        std::unique_ptr<Object> obj;
        if (subChance > 0.6f) {
          obj = std::make_unique<Object>(
              ModelManager::getModel(ModelName::TREE_1));
          obj->setScale(0.006f);
        } else if (subChance > 0.4f) {
          obj = std::make_unique<Object>(
              ModelManager::getModel(ModelName::BUSH_2));
          obj->setScale(0.15f);
          custom_position_offset = {0, 0.80f, -0.3};
        } else if (subChance > 0.25f) {
          obj = std::make_unique<Object>(
              ModelManager::getModel(ModelName::TREE_2));
          obj->setScale(0.4f);
        } else if (subChance > 0.1f) {
          obj = std::make_unique<Object>(
              ModelManager::getModel(ModelName::BUSH_1));
          obj->setScale(0.002f);
        } else {
          obj = std::make_unique<Object>(
              ModelManager::getModel(ModelName::ROCK_1));
          obj->setScale(0.005f);
        }

        float xOffset = ((float)rand() / RAND_MAX - 0.5f) * 0.5f;
        obj->setPosition(
            glm::vec3((float)i + xOffset, row.getHeight(), row.getZ() - 0.25f) +
            custom_position_offset);
        obj->setRotation(glm::vec3(
            0.0f, ((float)rand() / RAND_MAX) * glm::two_pi<float>(), 0.0f));
        row.addObject(std::move(obj));
      }
    }
  };

  for (auto &row : m_map.getRows()) {
    if (row->getType() == RowType::GRASS) {
      populateGreenery(*row);
    }

    if (row->getType() == RowType::ROAD) {
      // Add moving cars to roads
      float direction = ((float)rand() / RAND_MAX > 0.5f) ? 1.0f : -1.0f;
      float speed = (2.0f + (float)rand() / RAND_MAX * 3.0f) * direction;

      // Spawn 1-2 cars per road
      int numCars = 1 + (rand() % 2);
      for (int c = 0; c < numCars; ++c) {
        float xPos = ((float)rand() / RAND_MAX - 0.5f) * 20.0f;
        auto car = std::make_unique<Car>(
            ModelManager::getModel(ModelName::CAR_1), speed);
        car->setPosition(
            glm::vec3(xPos, row->getHeight(), row->getZ() - 0.25f));
        car->setScale(0.2f);
        if (direction < 0.0f) {
          car->setRotation(glm::vec3(0.0f, glm::radians(180.0f), 0.0f));
        }
        row->addObject(std::move(car));
      }
    }
  }

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

void Game::update(double delta_time) { m_map.update(delta_time); }

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

  // Bind and draw Skybox first (as background)
  auto skyboxTex = TextureManager::getTexture(TextureName("skybox"));
  Shader &skybox_shader = ShaderManager::getShader(ShaderType::SKYBOX);
  glDepthMask(GL_FALSE);
  m_skybox->draw(camera, skybox_shader, *skyboxTex);
  glDepthMask(GL_TRUE);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glm::mat4 projection = camera.getProjectionMatrix();
  glm::mat4 view = camera.getViewMatrix();

  Shader &pbr_shader = ShaderManager::getShader(ShaderType::PBR);
  pbr_shader.use();
  pbr_shader.setMat4("u_Projection", projection);
  pbr_shader.setMat4("u_View", view);
  pbr_shader.setVec3("u_CameraPos", camera.Position);
  pbr_shader.setMat4("u_LightSpaceMatrix", m_lightSpaceMatrix);

  // Bind Skybox for reflections
  glBindTextureUnit(10, skyboxTex->getTexID());
  pbr_shader.setInt("u_Skybox", 10);

  // Bind Shadow Map
  glBindTextureUnit(11, m_shadowMapTex);
  pbr_shader.setInt("u_ShadowMap", 11);

  // Lighting setup
  LightingManager::apply(pbr_shader);

  pbr_shader.setFloat("u_HeightScale", 0.05f);

  // Draw Map
  m_map.draw(pbr_shader);

  // Draw Player (Last for transparency blending)
  m_player->draw(pbr_shader);

  glDisable(GL_BLEND);
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
