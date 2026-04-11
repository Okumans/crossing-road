#include "core/app.hpp"

#include "glad/gl.h"

#include "GLFW/glfw3.h"
#include "glm/fwd.hpp"
#include "graphics/material.hpp"
#include "resource/material_manager.hpp"
#include "resource/model_manager.hpp"
#include "resource/shader_manager.hpp"
#include "resource/texture_manager.hpp"
#include "ui/ui_manager.hpp"
#include "utility/utility.hpp"

#ifdef EMBED_SHADER
#include "ui.frag.glsl.h"
#include "ui.vert.glsl.h"

static std::string arr_to_str(unsigned char *arr, unsigned int len) {
  return std::string(reinterpret_cast<char *>(arr), len);
}
#endif

void App::render(double delta_time) {
  _handleProcessInput(delta_time);

  m_game.update(delta_time);
  // }

  // Update Camera to follow player with lerp for smoothness
  glm::vec3 playerPos = m_game.getPlayerPosition();
  glm::vec3 targetCameraPos =
      glm::vec3(0, playerPos.y, playerPos.z) + glm::vec3(8.0f, 8.0f, 8.0f);
  float lerpFactor = 5.0f;
  m_camera.Position = glm::mix(m_camera.Position, targetCameraPos,
                               (float)delta_time * lerpFactor);

  _updateUIElements(delta_time);

  m_game.render(delta_time, m_camera);
  m_uiManager.render(m_appState.windowWidth, m_appState.windowHeight);
}

App::App(GLFWwindow *window)
    : m_window(window), m_camera(glm::vec3(8.25f, 8.0f, 8.25f)) {

  m_camera.setPitch(-35.264f);
  m_camera.setYaw(-107.25f);
  m_camera.Zoom = 30.0f;

  glfwSetWindowUserPointer(m_window, (void *)this);

  glfwSetKeyCallback(m_window, _glfwKeyCallback);
  glfwSetCursorPosCallback(m_window, _glfwMouseMoveCallback);
  glfwSetMouseButtonCallback(m_window, _glfwMouseButtonCallback);
  glfwSetScrollCallback(m_window, _glfwScrollCallback);
  glfwSetFramebufferSizeCallback(m_window, _glfwFramebufferSizeCallback);

  _setupResources();
  _setupUIElements();

  int width, height;
  glfwGetWindowSize(m_window, &width, &height);

  m_appState.windowWidth = width;
  m_appState.windowHeight = height;

  m_camera.updateSceneSize(width, height);
}

App::~App() = default;

void App::_setupResources() {
  // Load Shader
#ifdef EMBED_SHADER
  ShaderManager::loadShaderSource(
      ShaderType::UI, arr_to_str(ui_vert_glsl, ui_vert_glsl_len).c_str(),
      arr_to_str(ui_frag_glsl, ui_frag_glsl_len).c_str());
#else
  ShaderManager::loadShader(ShaderType::UI, UI_VERTEX_SHADER_PATH,
                            UI_FRAGMENT_SHADER_PATH);
  ShaderManager::loadShader(ShaderType::PBR, SHADER_PATH "/pbr.vert.glsl",
                            SHADER_PATH "/pbr.frag.glsl");
  ShaderManager::loadShader(ShaderType::SKYBOX, SHADER_PATH "/skybox.vert.glsl",
                            SHADER_PATH "/skybox.frag.glsl");
  ShaderManager::loadShader(ShaderType::SHADOW, SHADER_PATH "/shadow.vert.glsl",
                            SHADER_PATH "/shadow.frag.glsl");
  ShaderManager::loadShader(ShaderType::IRRADIANCE,
                            SHADER_PATH "/irradiance.vert.glsl",
                            SHADER_PATH "/irradiance.frag.glsl");
  ShaderManager::loadShader(ShaderType::WATER, SHADER_PATH "/pbr.vert.glsl",
                            SHADER_PATH "/water.frag.glsl");
  ShaderManager::loadShader(ShaderType::DEBUG, SHADER_PATH "/debug.vert.glsl",
                            SHADER_PATH "/debug.frag.glsl");
#endif

  // Ensure the existence of static generated textures
  if (!TextureManager::exists(STATIC_BLACK_TEXTURE))
    TextureManager::manage(STATIC_BLACK_TEXTURE,
                           TextureManager::generateStaticBlackTexture());

  if (!TextureManager::exists(STATIC_WHITE_TEXTURE))
    TextureManager::manage(STATIC_WHITE_TEXTURE,
                           TextureManager::generateStaticWhiteTexture());

  if (!TextureManager::exists(STATIC_NORMAL_TEXTURE))
    TextureManager::manage(STATIC_NORMAL_TEXTURE,
                           TextureManager::generateStaticNormalTexture());

  if (!TextureManager::exists(STATIC_PBR_DEFAULT_TEXTURE))
    TextureManager::manage(STATIC_PBR_DEFAULT_TEXTURE,
                           TextureManager::generateStaticPBRDefaultTexture());

  // Load the models
  ModelManager::loadModel(ModelName::CHICKEN,
                          ASSETS_PATH "/objects/chicken/chicken.glb");
  ModelManager::loadModel(ModelName::TREE_1,
                          ASSETS_PATH "/objects/tree/tree_1.glb");
  ModelManager::loadModel(ModelName::TREE_2,
                          ASSETS_PATH "/objects/tree/tree_2.glb");
  ModelManager::loadModel(ModelName::BUSH_1,
                          ASSETS_PATH "/objects/tree/bush_1.glb");
  ModelManager::loadModel(ModelName::BUSH_2,
                          ASSETS_PATH "/objects/tree/bush_2.glb");
  ModelManager::loadModel(ModelName::ROCK_1,
                          ASSETS_PATH "/objects/rock/rock_1.glb");
  ModelManager::loadModel(ModelName::CAR_1,
                          ASSETS_PATH "/objects/car/car_1.glb");
  ModelManager::loadModel(ModelName::CAR_2,
                          ASSETS_PATH "/objects/car/car_2.glb");
  ModelManager::loadModel(ModelName::TRAIN_1,
                          ASSETS_PATH "/objects/car/train_1.glb");

  // Load the material & Textures, for the ground textures
  // TODO: Use enum instead of fixed string
  loadMaterialFolder("grass_1", ASSETS_PATH "/textures/grass/1");
  loadMaterialFolder("grass_2", ASSETS_PATH "/textures/grass/2");
  loadMaterialFolder("road_1", ASSETS_PATH "/textures/road/3");
  loadMaterialFolder("road_2", ASSETS_PATH "/textures/road/2");

  loadMaterialFolder("water_1", ASSETS_PATH "/textures/water");
  MaterialManager::addMaterial(
      "water_1", Material::builder(MaterialManager::getMaterial("water_1"))
                     .setRoughnessFactor(0.07f)
                     .setMetallicFactor(0.0f)
                     .create());

  // Load other resources
  m_game.setup();
  // m_game.toggleDebugAABB();
  m_font.loadDefaultFont();
}

void App::_setupUIElements() {
  m_uiManager.addTextElement("fps_counter", {1.0f, 1.0f, 0.0f, 0.0f}, "FPS: 0",
                             m_font, {1.0f, 1.0f, 1.0f, 1.0f}, 0.2f);
}

void App::_updateUIElements(double delta_time) {
  static double timeAccumulator = 0.0;
  static int frameCount = 0;
  static double lastFps = 0.0;

  timeAccumulator += delta_time;
  frameCount++;

  if (timeAccumulator >= 0.5) {
    lastFps = frameCount / timeAccumulator;
    timeAccumulator = 0.0;
    frameCount = 0;

    auto *fpsElement =
        dynamic_cast<TextElement *>(m_uiManager.getElement("fps_counter"));
    if (fpsElement) {
      fpsElement->text = std::format("FPS: {}", static_cast<int>(lastFps));
    }
  }
}

void App::_handleProcessInput(double delta_time) {
  if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) {
    m_game.moveLeft(delta_time);
  }

  if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) {
    m_game.moveRight(delta_time);
  }
}

void App::_handleKeyCallback(int key, int scancode, int action, int mods) {
  if (action != GLFW_PRESS)
    return;

  if (key == GLFW_KEY_SPACE) {
    m_game.moveForward();
  }
}

// internal event handler
void App::_handleMouseMoveCallback(double pos_x, double pos_y) {
  m_appState.inputState.mouseLastX = pos_x;
  m_appState.inputState.mouseLastY = pos_y;
}

void App::_handleMouseClickCallback(int button, int action, int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    m_uiManager.handleClick(m_appState.inputState.mouseLastX,
                            m_appState.inputState.mouseLastY);
  }
}

void App::_handleScrollCallback(double offset_x, double offset_y) {}

void App::_handleFramebufferSizeCallback(int width, int height) {
  glViewport(0, 0, width, height);
  m_appState.windowWidth = width;
  m_appState.windowHeight = height;
  m_camera.updateSceneSize(width, height);
}

// GLFW static callbacks adapters
void App::_glfwKeyCallback(GLFWwindow *window, int key, int scancode,
                           int action, int mods) {
  App *app = static_cast<App *>(glfwGetWindowUserPointer(window));

  if (app) {
    app->_handleKeyCallback(key, scancode, action, mods);
  }
}

void App::_glfwMouseMoveCallback(GLFWwindow *window, double x_pos,
                                 double y_pos) {
  App *app = static_cast<App *>(glfwGetWindowUserPointer(window));

  if (app) {
    app->_handleMouseMoveCallback(x_pos, y_pos);
  }
}

void App::_glfwMouseButtonCallback(GLFWwindow *window, int button, int action,
                                   int mods) {
  App *app = static_cast<App *>(glfwGetWindowUserPointer(window));

  if (app) {
    app->_handleMouseClickCallback(button, action, mods);
  }
}

void App::_glfwScrollCallback(GLFWwindow *window, double offset_x,
                              double offset_y) {
  App *app = static_cast<App *>(glfwGetWindowUserPointer(window));

  if (app) {
    app->_handleScrollCallback(offset_x, offset_y);
  }
}

void App::_glfwFramebufferSizeCallback(GLFWwindow *window, int width,
                                       int height) {
  App *app = static_cast<App *>(glfwGetWindowUserPointer(window));

  if (app) {
    app->_handleFramebufferSizeCallback(width, height);
  }
}
