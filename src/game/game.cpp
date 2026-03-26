#include "game.hpp"
#include "glm/trigonometric.hpp"
#include "graphics/material.hpp"
#include "resource/model_manager.hpp"
#include "resource/shader_manager.hpp"

#include "glad/gl.h"
#include "glm/fwd.hpp"
#include "resource/texture_manager.hpp"
#include <memory>

Game::Game() : m_player(nullptr), m_skybox(std::make_unique<Skybox>()) {}

void Game::setup() {
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

  auto roadTexDiffuse = TextureManager::getTexture(TextureName("road_diffuse"));
  auto roadTexAO = TextureManager::getTexture(TextureName("road_ao"));
  auto roadTexNormal = TextureManager::getTexture(TextureName("road_normal"));
  auto roadTexHeight = TextureManager::getTexture(TextureName("road_height"));
  auto roadTexRoughness =
      TextureManager::getTexture(TextureName("road_roughness"));

  auto grassTexDiffuse =
      TextureManager::getTexture(TextureName("grass_diffuse"));
  auto grassTexAO = TextureManager::getTexture(TextureName("grass_ao"));
  auto grassTexNormal = TextureManager::getTexture(TextureName("grass_normal"));
  auto grassTexHeight = TextureManager::getTexture(TextureName("grass_height"));
  auto grassTexRoughness =
      TextureManager::getTexture(TextureName("grass_roughness"));

  auto waterTex = TextureManager::getTexture(TextureName("water"));

  const Material grass_mat = Material::builder()
                                 .setDiffuse(grassTexDiffuse)
                                 .setNormal(grassTexNormal)
                                 .setHeight(grassTexHeight)
                                 .setRoughness(grassTexRoughness)
                                 .setAO(grassTexAO)
                                 .create();

  const Material road_mat = Material::builder()
                                .setDiffuse(roadTexDiffuse)
                                .setNormal(roadTexNormal)
                                .setHeight(roadTexHeight)
                                .setRoughness(roadTexRoughness)
                                .setAO(roadTexAO)
                                .create();

  const Material water_mat = Material::builder().setDiffuse(waterTex).create();

  m_map.addRow(RowType::GRASS, grass_mat);
  m_map.addRow(RowType::ROAD, road_mat);
  m_map.addRow(RowType::ROAD, road_mat);
  m_map.addRow(RowType::GRASS, grass_mat);
  m_map.addRow(RowType::WATER, water_mat);

  m_player =
      std::make_unique<Object>(ModelManager::getModel(ModelName::CHICKEN));
  m_player->setRotation({0, glm::radians(-90.0f), 0});
  m_player->setPosition({0.25f, 0.0f, 0.25f});
}

void Game::update(double delta_time) { ; }

void Game::render(double delta_time, Camera &camera) {
  glEnable(GL_DEPTH_TEST);

  glm::mat4 projection = camera.getProjectionMatrix();
  glm::mat4 view = camera.getViewMatrix();

  Shader &pbr_shader = ShaderManager::getShader(ShaderType::PBR);
  pbr_shader.use();
  pbr_shader.setMat4("u_Projection", projection);
  pbr_shader.setMat4("u_View", view);
  pbr_shader.setVec3("u_CameraPos", camera.Position);

  // Bind Skybox
  auto skyboxTex = TextureManager::getTexture(TextureName("skybox"));
  glBindTextureUnit(10, skyboxTex->getTexID());
  pbr_shader.setInt("u_Skybox", 10);

  // Lighting setup
  pbr_shader.setInt("u_NumLights", 4);

  // 1. "Sun" Light (Far away, bright)
  pbr_shader.setVec3("u_Lights[0].position", glm::vec3(20.0f, 50.0f, 30.0f));
  pbr_shader.setVec3("u_Lights[0].color", glm::vec3(400.0f, 380.0f, 350.0f));

  // 2. Sky Blue Fill Light (From above)
  pbr_shader.setVec3("u_Lights[1].position", glm::vec3(-10.0f, 20.0f, 0.0f));
  pbr_shader.setVec3("u_Lights[1].color", glm::vec3(80.0f, 100.0f, 150.0f));

  // 3. Ground Bounce Fill (Warm)
  pbr_shader.setVec3("u_Lights[2].position", glm::vec3(0.0f, -5.0f, 0.0f));
  pbr_shader.setVec3("u_Lights[2].color", glm::vec3(50.0f, 40.0f, 30.0f));

  // 4. Dynamic Rotating Light (To see PBR highlights)
  static float lightTimer = 0.0f;
  lightTimer += (float)delta_time;
  glm::vec3 playerPos = m_player->getPosition();
  glm::vec3 lightPos = playerPos + glm::vec3(sin(lightTimer) * 5.0f, 2.0f,
                                             cos(lightTimer) * 5.0f);
  pbr_shader.setVec3("u_Lights[3].position", lightPos);
  pbr_shader.setVec3(
      "u_Lights[3].color",
      glm::vec3(150.0f, 150.0f, 150.0f)); // Bright white/grey light

  // Default PBR factors for things without textures
  pbr_shader.setFloat("u_MetallicFactor", 1.0f);
  pbr_shader.setFloat("u_RoughnessFactor", 1.0f);
  pbr_shader.setFloat("u_HeightScale",
                      0.05f); // Controls depth of parallax mapping

  // Draw Map
  m_map.draw(pbr_shader);

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
