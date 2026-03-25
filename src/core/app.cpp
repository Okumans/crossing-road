#include "core/app.hpp"
#include "GLFW/glfw3.h"
#include "glad/gl.h"
#include "ui/ui_manager.hpp"
#include "utility/shader_manager.hpp"

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

App::App(GLFWwindow *window)
    : m_window(window), m_camera(glm::vec3(0.0f, 0.0f, 3.0f)) {
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

  m_camera.UpdateSceneSize(width, height);
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
#endif

  m_font.loadDefaultFont();
}

void App::_setupUIElements() { ; }

void App::_updateUIElements() { ; }

void App::_handleProcessInput(double delta_time) {}

void App::_handleKeyCallback(int key, int scancode, int action, int mods) {}

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
  m_camera.UpdateSceneSize(width, height);
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
