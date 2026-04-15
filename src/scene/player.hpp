#pragma once

#include "game/row_queue.hpp"
#include "graphics/idrawable.hpp"
#include "resource/model_manager.hpp"
#include "scene/row_object.hpp"
#include <cstdint>

class Player : private RowObject, public IDrawable {
private:
  uint32_t m_currRowIdx;
  float m_z;

  // States for jump
  bool m_isJumping = false;
  float m_jumpTimer = 0.0f;
  const float m_jumpDuration = 0.40f;
  const float m_jumpHeight = 1.5f;
  glm::vec3 m_startPos;
  glm::vec3 m_targetPos;

public:
  Player(RowObject &&player_base_object, uint32_t curr_row_idx = 0)
      : RowObject(std::move(player_base_object)), m_currRowIdx(curr_row_idx) {}

  bool canJumpForward() { return !m_isJumping; }

  void jumpForward(uint32_t target_row_idx) {
    if (!canJumpForward())
      return;

    if (!RowQueue::get().exists(target_row_idx))
      return;

    m_isJumping = true;
    m_jumpTimer = 0.0f;
    m_startPos = getPosition();

    const Row *target_row = RowQueue::get().getRow(target_row_idx);

    m_targetPos.x = m_startPos.x;
    m_targetPos.z =
        RowQueue::get().getZ(target_row_idx) - (target_row->getDepth() / 2.0f);
    m_targetPos.y = target_row->getHeight();

    m_currRowIdx = target_row_idx;
  }

  void update(double delta_time) override {
    if (!m_isJumping) {
      return;
    }

    m_jumpTimer += static_cast<float>(delta_time);
    float t = m_jumpTimer / m_jumpDuration;

    // Jump finished
    if (t >= 1.0f) {
      t = 1.0f;
      m_isJumping = false;
    }

    // Linear interpolation for X and Z
    float current_z = std::lerp(m_startPos.z, m_targetPos.z, t);

    // Base terrain interpolation for Y
    float base_y = std::lerp(m_startPos.y, m_targetPos.y, t);

    // Parabolic Arc for Y
    float arc_y = 4.0f * m_jumpHeight * t * (1.0f - t);
    float current_y = base_y + arc_y;

    // Apply the newly calculated positions
    setPosition(glm::vec2(getPosition().x, current_y));
    m_z = current_z;
  }

  virtual void draw(const RenderContext &ctx) override {
    RowObject::draw(ctx, m_z);
  }

  void setPosition(glm::vec2 xy) { RowObject::setPosition(xy); }
  void setRotationY(float degrees) { RowObject::setRotationY(degrees); }
  void setRowIdx(uint32_t row_idx) {
    m_currRowIdx = row_idx;
    m_z = RowQueue::get().getZ(m_currRowIdx) -
          (RowQueue::get().getRow(m_currRowIdx)->getDepth() / 2.0f);
  }

  const glm::vec3 getRotation() const { return RowObject::getRotation(); }
  const glm::vec3 getPosition() const { return RowObject::getPosition(m_z); }
  const glm::vec3 getScale() const { return RowObject::getScale(); }
  const AABB &getWorldAABB() const { return RowObject::getWorldAABB(m_z); }
  const AABB &getLocalAABB() const { return RowObject::getLocalAABB(); }

  static Player getDefault() {
    RowObject object(ModelManager::getModel(ModelName::CHICKEN));

    object.setIncludeYRotationInAABB(true);
    object.setRotationY(-90.0f);
    object.setPosition({0.25f, 0.0f});

    return Player(std::move(object));
  }
};
