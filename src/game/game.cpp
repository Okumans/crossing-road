#include "game.hpp"
#include "game/map_manager.hpp"
#include "game/row.hpp"
#include "game/row_queue.hpp"
#include "glm/trigonometric.hpp"
#include "graphics/debug_drawer.hpp"
#include "graphics/ibl_generator.hpp"
#include "graphics/idrawable.hpp"
#include "graphics/shader.hpp"
#include "resource/lighting_manager.hpp"
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

  // Create player first to define collision height clip
  m_player =
      std::make_unique<RowObject>(ModelManager::getModel(ModelName::CHICKEN));
  m_player->setRotation({0, glm::radians(-90.0f), 0});
  m_player->setPosition({0.25f, 0.0f});

  // Set global collision clip based on player's height
  RowObject::s_useClipY = true;
  RowObject::s_minClipY = m_player->getLocalAABB().min.y;
  RowObject::s_maxClipY = m_player->getLocalAABB().max.y;

  // Load Skybox
  TextureManager::loadCubemap(
      TextureName("skybox"),
      {
          ASSETS_PATH "/textures/skybox/sky_3/px.hdr", // +X
          ASSETS_PATH "/textures/skybox/sky_3/nx.hdr", // -X
          ASSETS_PATH "/textures/skybox/sky_3/py.hdr", // +Y
          ASSETS_PATH "/textures/skybox/sky_3/ny.hdr", // -Y
          ASSETS_PATH "/textures/skybox/sky_3/pz.hdr", // +Z
          ASSETS_PATH "/textures/skybox/sky_3/nz.hdr"  // -Z
      });

  // Generate Irradiance Map
  auto skybox_tex = TextureManager::getTexture(TextureName("skybox"));
  m_skybox->setTexture(skybox_tex);

  auto &irradiance_shader = ShaderManager::getShader(ShaderType::IRRADIANCE);
  auto irradiance_map = IBLGenerator::generateIrradianceMap(
      *skybox_tex, *m_skybox, irradiance_shader);
  TextureManager::manage(TextureName("irradiance_map"),
                         std::move(*irradiance_map));

  m_map.addTerrain(TerrainType::GRASSY);
  m_map.addTerrain(TerrainType::ROAD);
  m_map.addTerrain(TerrainType::HILLY);
  m_map.addTerrain(TerrainType::ROAD);
  m_map.addTerrain(TerrainType::GRASSY);
  m_map.addTerrain(TerrainType::ROAD);
  m_map.addTerrain(TerrainType::ROAD);
  m_map.addTerrain(TerrainType::HILLY);
  m_map.addTerrain(TerrainType::ROAD);
  m_map.addTerrain(TerrainType::GRASSY);

  // Setup Lights
  LightingManager::clearLights();

  // 1. "Sun" Light (Directional, casts shadows)
  LightingManager::addLight({.type = LightType::DIRECTIONAL,
                             .position = glm::vec3(0.3f, -1.0f, 0.1f),
                             .color = glm::vec3(11.0f, 9.0f, 8.0f) * .8f,
                             .castsShadows = true});

  // 2. Sky Blue Fill Light (Directional)
  LightingManager::addLight({.type = LightType::DIRECTIONAL,
                             .position = glm::vec3(0.5f, -1.0f, 0.2f),
                             .color = glm::vec3(0.1f, 0.15f, 0.25f)});

  // 3. Ground Bounce Fill (Point light)
  LightingManager::addLight({.type = LightType::POINT,
                             .position = glm::vec3(0.0f, -5.0f, 0.0f),
                             .color = glm::vec3(0.3f, 0.2f, 0.1f) * 5.0f});
}

void Game::update(double delta_time) {
  m_currentTime += static_cast<float>(delta_time);

  // --- PBR DEBUG: Rotate Sun ---
  float sun_speed = 0.3f;
  float sun_radius = 1.0f;
  glm::vec3 sun_dir = glm::normalize(
      glm::vec3(std::cos(m_currentTime * sun_speed) * sun_radius, -1.0f,
                std::sin(m_currentTime * sun_speed) * sun_radius));

  Light sun = LightingManager::getShadowCaster();
  sun.position = sun_dir;
  LightingManager::setLight(0, sun);
  // -----------------------------

  m_map.update(delta_time);
}

void Game::render(double delta_time, Camera &camera) {
  glEnable(GL_DEPTH_TEST);

  const Row *curr_row = RowQueue::get().getRow(m_playerRowIdx);
  float player_z =
      RowQueue::get().getZ(m_playerRowIdx) - curr_row->getDepth() / 2.0f;
  m_player->setPosition({m_player->getPosition().x, curr_row->getHeight()});

  // 1. Shadow Pass
  m_lightSpaceMatrix = LightingManager::calculateLightSpaceMatrix(
      m_player->getPosition(RowQueue::get().getZ(m_playerRowIdx)));

  Shader &shadow_shader = ShaderManager::getShader(ShaderType::SHADOW);
  shadow_shader.use();
  shadow_shader.setMat4("u_LightSpaceMatrix", m_lightSpaceMatrix);

  glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
  glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFBO);
  glClear(GL_DEPTH_BUFFER_BIT);
  glDisable(GL_CULL_FACE);

  {
    RenderContext shadow_draw_ctx = {
        .shader = shadow_shader, .camera = camera, .deltaTime = delta_time};

    m_map.draw(shadow_draw_ctx);
    m_player->draw(shadow_draw_ctx, player_z);
  }

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
  pbr_shader.setInt("u_SpecularEnvMap", 10);

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
  pbr_shader.setFloat("u_AmbientIntensity", 1.0f);

  // Update water shader global uniforms
  Shader &water_shader = ShaderManager::getShader(ShaderType::WATER);
  water_shader.use();
  water_shader.setMat4("u_Projection", projection);
  water_shader.setMat4("u_View", view);
  water_shader.setVec3("u_CameraPos", camera.Position);
  water_shader.setMat4("u_LightSpaceMatrix", m_lightSpaceMatrix);
  water_shader.setInt("u_SpecularEnvMap", 10);
  water_shader.setInt("u_IrradianceMap", 12);
  water_shader.setInt("u_ShadowMap", 11);
  water_shader.setFloat("u_Time", m_currentTime);
  LightingManager::apply(water_shader);
  water_shader.setFloat("u_AmbientIntensity", 1.0f);

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
