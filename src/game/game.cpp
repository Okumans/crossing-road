#include "game.hpp"
#include "game/map_manager.hpp"
#include "game/row_queue.hpp"
#include "glm/trigonometric.hpp"
#include "graphics/debug_drawer.hpp"
#include "graphics/ibl_generator.hpp"
#include "graphics/material.hpp"
#include "graphics/shader.hpp"
#include "resource/lighting_manager.hpp"
#include "resource/material_manager.hpp"
#include "resource/model_manager.hpp"
#include "resource/shader_manager.hpp"
#include "resource/texture_manager.hpp"
#include "scene/row_object.hpp"

#include <glad/gl.h>
#include <glm/fwd.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>

Game::Game()
    : m_player(nullptr), m_skybox(std::make_unique<Skybox>()),
      m_shadowMapFBO(0), m_shadowMapTex(0) {}

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

  float border_color[] = {1.0, 1.0, 1.0, 1.0};

  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);

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
          ASSETS_PATH "/textures/skybox/pz.png", // +X (Right)
          ASSETS_PATH "/textures/skybox/nz.png", // -X (Left)
          ASSETS_PATH "/textures/skybox/py.png", // +Y (Top)
          ASSETS_PATH "/textures/skybox/ny.png", // -Y (Bottom)
          ASSETS_PATH "/textures/skybox/nx.png", // +Z (Back)
          ASSETS_PATH "/textures/skybox/px.png"  // -Z (Front)
      });

  // Generate Irradiance Map
  auto skybox_tex = TextureManager::getTexture(TextureName("skybox"));
  m_skybox->setTexture(skybox_tex);

  auto &irradiance_shader = ShaderManager::getShader(ShaderType::IRRADIANCE);
  auto irradiance_map = IBLGenerator::generateIrradianceMap(
      *skybox_tex, *m_skybox, irradiance_shader);
  TextureManager::manage(TextureName("irradiance_map"),
                         std::move(*irradiance_map));

  const Material &grass_mat_1 = MaterialManager::getMaterial("grass_1");
  const Material &grass_mat_2 = MaterialManager::getMaterial("grass_2");
  const Material &road_mat_1 = MaterialManager::getMaterial("road_1");
  const Material &road_mat_2 = MaterialManager::getMaterial("road_2");

  const Material &water_mat = MaterialManager::getMaterial("water_1");
  Shader &water_shader = ShaderManager::getShader(ShaderType::WATER);

  const float default_depth = 1.0f;

  // Start with some grass
  m_map.addTerrain(TerrainType::GRASSY);
  m_map.addTerrain(TerrainType::ROAD);
  m_map.addTerrain(TerrainType::HILLY);
  m_map.addTerrain(TerrainType::GRASSY);
  m_map.addTerrain(TerrainType::ROAD);
  m_map.addTerrain(TerrainType::HILLY);
  m_map.addTerrain(TerrainType::GRASSY);
  // m_map.addTerrain(std::make_unique<TextureRow>(RowType::GRASS, grass_mat_2,
  //                                           default_depth, 0.0f));
  // m_map.addTerrain(std::make_unique<TextureRow>(RowType::GRASS, grass_mat_1,
  //                                           default_depth, 0.0f));
  //
  // // A road
  // m_map.addTerrain(std::make_unique<TextureRow>(RowType::ROAD, road_mat_1,
  //                                           default_depth, 0.05f));
  // m_map.addTerrain(std::make_unique<TextureRow>(RowType::ROAD, road_mat_2,
  //                                           default_depth, 0.05f));
  // m_map.addTerrain(std::make_unique<TextureRow>(RowType::ROAD, road_mat_1,
  //                                           default_depth, 0.05f));
  // // More grass
  // m_map.addTerrain(std::make_unique<TextureRow>(RowType::GRASS, grass_mat_2,
  //                                           default_depth, 0.0f));
  //
  // // A river (recessed)
  // m_map.addTerrain(std::make_unique<TextureRow>(RowType::GRASS, grass_mat_1,
  //                                           default_depth, 0.0f,
  //                                           road_mat_2));
  // m_map.addTerrain(std::make_unique<WaterRow>(water_mat, water_shader,
  //                                         default_depth, -0.2f));
  // m_map.addTerrain(std::make_unique<WaterRow>(water_mat, water_shader,
  //                                         default_depth, -0.2f));
  // m_map.addTerrain(std::make_unique<TextureRow>(RowType::GRASS, grass_mat_1,
  //                                           default_depth, 0.0f,
  //                                           road_mat_2));
  //
  // // A raised grass area
  // m_map.addTerrain(std::make_unique<TextureRow>(RowType::GRASS, grass_mat_2,
  //                                           default_depth, 0.1f));
  // m_map.addTerrain(std::make_unique<TextureRow>(RowType::GRASS, grass_mat_1,
  //                                           default_depth, 0.1f));
  // m_map.addTerrain(std::make_unique<TextureRow>(RowType::GRASS, grass_mat_2,
  //                                           default_depth, 0.0f));
  //
  // // Another road
  // m_map.addTerrain(std::make_unique<TextureRow>(RowType::ROAD, road_mat_2,
  //                                           default_depth, 0.05f));
  // m_map.addTerrain(std::make_unique<TextureRow>(RowType::ROAD, road_mat_1,
  //                                           default_depth, 0.05f));
  //
  // // End with grass
  // m_map.addTerrain(std::make_unique<TextureRow>(RowType::GRASS, grass_mat_1,
  //                                           default_depth, 0.0f));
  // m_map.addTerrain(std::make_unique<TextureRow>(RowType::GRASS, grass_mat_2,
  //                                           default_depth, 0.0f));

  // Helper to add random greenery to a grass row (very sparse)
  // auto populateGreenery = [&](Row &row) {
  //   for (int i = -10; i <= 10; ++i) {
  //     if (std::abs(i) <= 1)
  //       continue; // Keep center clear for player
  //
  //     float chance = (float)rand() / RAND_MAX;
  //     glm::vec3 custom_position_offset = glm::vec3(0.0f);
  //
  //     if (chance > 0.95f) { // ~5% chance
  //       float subChance = (float)rand() / RAND_MAX;
  //       std::unique_ptr<Object> obj;
  //       if (subChance > 0.6f) {
  //         obj = std::make_unique<Object>(
  //             ModelManager::getModel(ModelName::TREE_1));
  //         obj->setScale(0.006f);
  //       } else if (subChance > 0.4f) {
  //         obj = std::make_unique<Object>(
  //             ModelManager::getModel(ModelName::BUSH_2));
  //         obj->setScale(0.0025f);
  //         custom_position_offset = {0, 0.20f, 0};
  //       } else if (subChance > 0.25f) {
  //         obj = std::make_unique<Object>(
  //             ModelManager::getModel(ModelName::TREE_2));
  //         custom_position_offset = {0, -0.10f, 0};
  //         obj->setScale(0.4f);
  //       } else if (subChance > 0.1f) {
  //         obj = std::make_unique<Object>(
  //             ModelManager::getModel(ModelName::BUSH_1));
  //         obj->setScale(0.002f);
  //       } else {
  //         obj = std::make_unique<Object>(
  //             ModelManager::getModel(ModelName::ROCK_1));
  //         obj->setScale(0.005f);
  //       }
  //
  //       float xOffset = ((float)rand() / RAND_MAX - 0.5f) * 0.5f;
  //       obj->setPosition(glm::vec3((float)i + xOffset, row.getHeight(),
  //                                  row.getZPos() - (row.getDepth() - 0.25f))
  //                                  +
  //                        custom_position_offset);
  //       obj->setRotation(glm::vec3(
  //           0.0f, ((float)rand() / RAND_MAX) * glm::two_pi<float>(), 0.0f));
  //       row.addObject(std::move(obj));
  //     }
  //   }
  // };
  //
  // for (Row *row : m_map.getRows()) {
  //   if (row->getType() == RowType::GRASS) {
  //     populateGreenery(*row);
  //   }
  //
  //   if (row->getType() == RowType::ROAD) {
  //     float direction = ((float)rand() / RAND_MAX > 0.5f) ? 1.0f : -1.0f;
  //
  //     if ((float)rand() / RAND_MAX <
  //         0.4f) { // Increased spawn chance for trains
  //       float speed = (6.0f + (float)rand() / RAND_MAX * 4.0f) * direction;
  //       float xPos = direction > 0.0f ? -15.0f : 15.0f;
  //       auto train = std::make_unique<Car>(
  //           ModelManager::getModel(ModelName::TRAIN_1), speed);
  //       train->setRotation({0, glm::radians(90.0f), 0});
  //       train->setPosition(
  //           glm::vec3(xPos, row->getHeight() + 0.3f,
  //                     row->getZPos() - (row->getDepth() - 0.25f)));
  //       train->setScale(0.3f);
  //       if (direction < 0.0f) {
  //         train->setRotation(glm::vec3(0.0f, glm::radians(-90.0f), 0.0f));
  //       }
  //       row->addObject(std::move(train));
  //     } else {
  //       float speed = (1.5f + (float)rand() / RAND_MAX * 3.0f) * direction;
  //       int numCars = 1 + (rand() % 2);
  //
  //       for (int c = 0; c < numCars; ++c) {
  //         float xPos = ((float)rand() / RAND_MAX - 0.5f) * 20.0f;
  //
  //         if (rand() % 2 == 0) {
  //           auto car = std::make_unique<Car>(
  //               ModelManager::getModel(ModelName::CAR_1), speed);
  //           car->setPosition(
  //               glm::vec3(xPos, row->getHeight(),
  //                         row->getZPos() - (row->getDepth() - 0.25f)));
  //           car->setScale(0.0030f);
  //           car->setRotation(glm::vec3(0, glm::radians(90.0f), 0));
  //
  //           if (direction < 0.0f) {
  //             car->rotate(glm::vec3(0.0f, glm::radians(180.0f), 0.0f));
  //           }
  //           row->addObject(std::move(car));
  //         } else {
  //           auto car = std::make_unique<Car>(
  //               ModelManager::getModel(ModelName::CAR_2), speed);
  //           car->setPosition(
  //               glm::vec3(xPos, row->getHeight(),
  //                         row->getZPos() - (row->getDepth() - 0.25f)));
  //           car->setScale(0.0022f);
  //           car->setRotation(glm::vec3(0, glm::radians(90.0f), 0));
  //
  //           if (direction < 0.0f) {
  //             car->rotate(glm::vec3(0.0f, glm::radians(180.0f), 0.0f));
  //           }
  //           row->addObject(std::move(car));
  //         }
  //       }
  //     }
  //   }
  // }

  m_player =
      std::make_unique<RowObject>(ModelManager::getModel(ModelName::CHICKEN));
  m_player->setRotation({0, glm::radians(-90.0f), 0});
  m_player->setPosition({0.25f, 0.0f});

  // Setup Lights
  LightingManager::clearLights();

  // 1. "Sun" Light (Directional, casts shadows)
  LightingManager::addLight({.type = LightType::DIRECTIONAL,
                             .position = glm::vec3(-1.0f, -0.6f, -0.2f),
                             .color = glm::vec3(12.0f, 11.0f, 10.0f),
                             .castsShadows = true});

  // 2. Sky Blue Fill Light (Directional)
  LightingManager::addLight({.type = LightType::DIRECTIONAL,
                             .position = glm::vec3(0.5f, -1.0f, 0.2f),
                             .color = glm::vec3(0.1f, 0.15f, 0.25f)});

  // 3. Ground Bounce Fill (Point light)
  LightingManager::addLight({.type = LightType::POINT,
                             .position = glm::vec3(0.0f, -5.0f, 0.0f),
                             .color = glm::vec3(0.15f, 0.1f, 0.05f)});
}

void Game::update(double delta_time) {
  m_currentTime += static_cast<float>(delta_time);
  m_map.update(delta_time);

  // Update sun position (first light)
  float angle = m_currentTime * 0.03f; // speed
  glm::vec3 sunDir = glm::vec3(cos(angle), -0.6f, sin(angle));

  Light sun = {.type = LightType::DIRECTIONAL,
               .position = sunDir,
               .color = glm::vec3(12.0f, 11.0f, 10.0f),
               .castsShadows = true};
  LightingManager::setLight(0, sun);
}

void Game::render(double delta_time, Camera &camera) {
  glEnable(GL_DEPTH_TEST);

  const Row *curr_row = RowQueue::get().getRow(m_playerRowIdx);
  float player_z =
      RowQueue::get().getZ(m_playerRowIdx) - curr_row->getDepth() / 2.0f;
  m_player->setPosition({m_player->getPosition().x, curr_row->getHeight()});

  // 1. Shadow Pass
  m_lightSpaceMatrix =
      LightingManager::calculateLightSpaceMatrix(m_player->getPosition(0.75));

  Shader &shadow_shader = ShaderManager::getShader(ShaderType::SHADOW);
  shadow_shader.use();
  shadow_shader.setMat4("u_LightSpaceMatrix", m_lightSpaceMatrix);

  glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
  glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFBO);
  glClear(GL_DEPTH_BUFFER_BIT);
  glDisable(GL_CULL_FACE);

  m_map.draw(
      {.shader = shadow_shader, .camera = camera, .deltaTime = delta_time});
  m_player->draw(
      {.shader = shadow_shader, .camera = camera, .deltaTime = delta_time},
      player_z);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glCullFace(GL_BACK); // Restore back-face culling

  // 2. Main Pass
  glViewport(0, 0, (int)camera.getSceneWidth(), (int)camera.getSceneHeight());
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Bind and draw Skybox first (as background)
  auto skyboxTex = TextureManager::getTexture(TextureName("skybox"));
  Shader &skybox_shader = ShaderManager::getShader(ShaderType::SKYBOX);
  glDepthMask(GL_FALSE);

  m_skybox->draw({
      .shader = skybox_shader,
      .camera = camera,
      .deltaTime = delta_time,
  });

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

  // Bind Irradiance Map for diffuse IBL
  auto irradianceMap =
      TextureManager::getTexture(TextureName("irradiance_map"));
  glBindTextureUnit(12, irradianceMap->getTexID());
  pbr_shader.setInt("u_IrradianceMap", 12);

  // Bind Shadow Map
  glBindTextureUnit(11, m_shadowMapTex);
  pbr_shader.setInt("u_ShadowMap", 11);

  // Lighting setup
  LightingManager::apply(pbr_shader);

  pbr_shader.setFloat("u_HeightScale", 0.03f);
  pbr_shader.setFloat("u_AOFactor", 1.0f);
  pbr_shader.setFloat("u_AmbientIntensity", 0.4f);

  // Update water shader global uniforms
  Shader &water_shader = ShaderManager::getShader(ShaderType::WATER);
  water_shader.use();
  water_shader.setMat4("u_Projection", projection);
  water_shader.setMat4("u_View", view);
  water_shader.setVec3("u_CameraPos", camera.Position);
  water_shader.setMat4("u_LightSpaceMatrix", m_lightSpaceMatrix);
  water_shader.setInt("u_Skybox", 10);
  water_shader.setInt("u_IrradianceMap", 12);
  water_shader.setInt("u_ShadowMap", 11);
  water_shader.setFloat("u_Time", m_currentTime);
  LightingManager::apply(water_shader);
  water_shader.setFloat("u_AmbientIntensity", 0.4f);

  // Return to PBR shader as default
  pbr_shader.use();

  // Draw Map
  pbr_shader.setVec3("u_BaseColor", glm::vec3(1.0f));
  m_map.draw({.shader = pbr_shader, .camera = camera, .deltaTime = delta_time});

  // Draw Player (Last for transparency blending)
  // Lower base color slightly to avoid "overblown" look under strong light
  pbr_shader.setVec3("u_BaseColor", glm::vec3(0.8f));
  pbr_shader.setVec2("u_UVOffset", glm::vec2(0.0f));
  m_player->draw(
      {.shader = pbr_shader, .camera = camera, .deltaTime = delta_time},
      player_z);
  pbr_shader.setVec3("u_BaseColor", glm::vec3(1.0f));

  if (m_debugAABB) {
    RenderContext debugCtx = {
        .shader = pbr_shader, .camera = camera, .deltaTime = delta_time};

    DebugDrawer::drawAABB(debugCtx, m_player->getWorldAABB(player_z),
                          {1.0f, 1.0f, 0.0f});

    if (curr_row) {
      float row_z = RowQueue::get().getZ(m_playerRowIdx);
      for (const auto &obj : curr_row->getObjects()) {

        float center_z = -(curr_row->getDepth() / 2.0f);
        float obj_z = row_z + center_z - obj->getWorldAABBCenter().z;

        DebugDrawer::drawAABB(debugCtx, obj->getWorldAABB(obj_z),
                              {1.0f, 0.0f, 0.0f});
      }
    }
  }

  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
}

void Game::moveForward() {
  if (m_player) {
    m_playerRowIdx++;
  }
}

void Game::moveLeft(double delta_time) {
  if (m_player) {
    glm::vec2 pos = m_player->getPosition();
    pos.x -= 2.0f * (float)delta_time;
    m_player->setPosition(pos);
  }
}

void Game::moveRight(double delta_time) {
  if (m_player) {
    glm::vec2 pos = m_player->getPosition();
    pos.x += 2.0f * (float)delta_time;
    m_player->setPosition(pos);
  }
}

glm::vec3 Game::getPlayerPosition() const {
  float z = RowQueue::get().getZ(m_playerRowIdx);

  if (m_player)
    return m_player->getPosition(z);
  return glm::vec3(0.0f);
}
