#include "core/app.hpp"
#include "GLFW/glfw3.h"
#include "glad/gl.h"
#include "resource/model_manager.hpp"
#include "resource/shader_manager.hpp"
#include "resource/texture_manager.hpp"
#include "ui/ui_manager.hpp"

#ifdef EMBED_SHADER
#include "ui.frag.glsl.h"
#include "ui.vert.glsl.h"

static std::string arr_to_str(unsigned char *arr, unsigned int len) {
  return std::string(reinterpret_cast<char *>(arr), len);
}
#endif

void App::render(double delta_time) {
  _handleProcessInput(delta_time);

  if (m_appState.gameStarted) {
    m_game.update(delta_time);
  }

  _updateUIElements();

  m_game.render(delta_time, m_camera);
  m_uiManager.render(m_appState.windowWidth, m_appState.windowHeight);
}

App::App(GLFWwindow *window) : m_window(window), m_camera(glm::vec3(8.0f)) {

  m_camera.setPitch(-35.264f);
  m_camera.setYaw(-107.25f);

  glfwSetWindowUserPointer(m_window, (void *)this);

  glfwSetKeyCallback(m_window, _glfwKeyCallback);
  glfwSetCursorPosCallback(m_window, _glfwMouseMoveCallback);
  glfwSetMouseButtonCallback(m_window, _glfwMouseButtonCallback);
  glfwSetScrollCallback(m_window, _glfwScrollCallback);
  glfwSetFramebufferSizeCallback(m_window, _glfwFramebufferSizeCallback);

  _setupResources();
  _setupUIElements();

  m_game.setup();

  int width, height;
  glfwGetWindowSize(m_window, &width, &height);

  m_camera.updateSceneSize(width, height);
}

App::~App() = default;

void App::_setupResources() {
#ifdef EMBED_SHADER
  ShaderManager::loadShaderSource(
      ShaderType::UI, arr_to_str(ui_vert_glsl, ui_vert_glsl_len).c_str(),
      arr_to_str(ui_frag_glsl, ui_frag_glsl_len).c_str());
#else
  ShaderManager::loadShader(ShaderType::UI, UI_VERTEX_SHADER_PATH,
                            UI_FRAGMENT_SHADER_PATH);
  ShaderManager::loadShader(ShaderType::CAMERA,
                            SHADER_PATH "/model_loading.vert.glsl",
                            SHADER_PATH "/model_loading.frag.glsl");
  ShaderManager::loadShader(ShaderType::PBR, SHADER_PATH "/pbr.vert.glsl",
                            SHADER_PATH "/pbr.frag.glsl");
  ShaderManager::loadShader(ShaderType::SKYBOX, SHADER_PATH "/skybox.vert.glsl",
                            SHADER_PATH "/skybox.frag.glsl");
#endif
  if (!TextureManager::exists(STATIC_BLACK_TEXTURE))
    TextureManager::manage(STATIC_BLACK_TEXTURE,
                           TextureManager::generateStaticBlackTexture());

  if (!TextureManager::exists(STATIC_WHITE_TEXTURE))
    TextureManager::manage(STATIC_WHITE_TEXTURE,
                           TextureManager::generateStaticWhiteTexture());

  if (!TextureManager::exists(STATIC_NORMAL_TEXTURE))
    TextureManager::manage(STATIC_NORMAL_TEXTURE,

                           TextureManager::generateStaticNormalTexture());
  ModelManager::loadModel(ModelName::CHICKEN,
                          ASSETS_PATH "/objects/chicken/chicken.glb");

  TextureManager::loadTexture(TextureName("grass_diffuse"),
                              TextureType::DIFFUSE,
                              ASSETS_PATH "/textures/grass_diffuse.jpg");
  TextureManager::loadTexture(TextureName("grass_height"), TextureType::HEIGHT,
                              ASSETS_PATH "/textures/grass_height.jpg");
  TextureManager::loadTexture(TextureName("grass_normal"), TextureType::NORMAL,
                              ASSETS_PATH "/textures/grass_normal.jpg");
  TextureManager::loadTexture(TextureName("grass_ao"), TextureType::AO,
                              ASSETS_PATH "/textures/grass_ao.jpg");
  TextureManager::loadTexture(TextureName("grass_roughness"),
                              TextureType::ROUGHNESS,
                              ASSETS_PATH "/textures/grass_roughness.jpg");

  TextureManager::loadTexture(TextureName("road_diffuse"), TextureType::DIFFUSE,
                              ASSETS_PATH "/textures/road.jpg");
  TextureManager::loadTexture(TextureName("road_height"), TextureType::HEIGHT,
                              ASSETS_PATH "/textures/road_height.jpg");
  TextureManager::loadTexture(TextureName("road_normal"), TextureType::NORMAL,
                              ASSETS_PATH "/textures/road_normal.jpg");
  TextureManager::loadTexture(TextureName("road_ao"), TextureType::AO,
                              ASSETS_PATH "/textures/road_ao.jpg");
  TextureManager::loadTexture(TextureName("road_roughness"),
                              TextureType::ROUGHNESS,
                              ASSETS_PATH "/textures/road_roughness.jpg");

  TextureManager::loadTexture(TextureName("water"), TextureType::DIFFUSE,
                              ASSETS_PATH "/textures/water.jpg");

  m_game.setup();
  m_font.loadDefaultFont();
}

void App::_setupUIElements() { ; }

void App::_updateUIElements() { ; }

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
