#pragma once

#include "game/row.hpp"

#include <algorithm>
#include <cstdint>
#include <deque>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <tuple>

class RowQueue {
private:
  static std::unique_ptr<RowQueue> s_instance;

  uint32_t m_currRowIdx;
  uint32_t m_firstStoredIdx;
  uint32_t m_capacity;
  float m_startZ;
  std::deque<Row *> m_storage;

  RowQueue(uint32_t capacity, float start_z)
      : m_currRowIdx(0), m_firstStoredIdx(0), m_capacity(capacity),
        m_startZ(start_z) {}

public:
  RowQueue(const RowQueue &) = delete;
  RowQueue &operator=(const RowQueue &) = delete;

  static void init(uint32_t capacity, float start_z) {
    if (!s_instance) {
      s_instance = std::unique_ptr<RowQueue>(new RowQueue(capacity, start_z));
    }
  }

  static void reset(uint32_t capacity, float start_z) {
    s_instance = std::unique_ptr<RowQueue>(new RowQueue(capacity, start_z));
  }

  static RowQueue &get() {
    if (!s_instance) {
      throw std::runtime_error("Please initialize RowQueue first");
    }
    return *s_instance;
  }

  uint32_t registerRow(Row *row) {
    m_storage.push_back(row);
    return m_currRowIdx++;
  }

  void step(uint32_t amount) {
    uint32_t toRemove =
        std::min(amount, static_cast<uint32_t>(m_storage.size()));
    for (uint32_t i = 0; i < toRemove; ++i) {
      m_startZ += m_storage[0]->getDepth();
      m_storage.pop_front();
    }
    m_firstStoredIdx += toRemove;
  }

  // verify if row exists in RowQueue
  bool exists(uint32_t row_idx) const {
    if (row_idx >= m_currRowIdx || row_idx < m_firstStoredIdx)
      return false;
    return true;
  }

  const Row *getRow(uint32_t row_idx) const {
    if (!exists(row_idx))
      return nullptr;
    return m_storage[row_idx - m_firstStoredIdx];
  }

  const float getZ(uint32_t row_idx) const {
    if (!exists(row_idx))
      return 0.0f;

    float acc_z = -m_startZ;
    for (uint32_t i = 0; i < row_idx - m_firstStoredIdx; ++i) {
      acc_z -= m_storage[i]->getDepth();
    }

    return acc_z;
  }

  bool isUnloadAllow(uint32_t row_idx) const {
    if (row_idx >= m_currRowIdx)
      return false;
    return !exists(row_idx);
  }

  auto getRows() const { return m_storage | std::views::take(m_capacity); }

  auto getRowsWithZ() const {
    return m_storage | std::views::take(m_capacity) | std::views::enumerate |
           std::views::transform([this](std::tuple<long, Row *> tup) {
             auto &[idx, row] = tup;
             return std::make_pair(
                 getZ(static_cast<uint32_t>(idx) + m_firstStoredIdx), row);
           });
  }

  uint32_t getCurrRowIdx() const { return m_currRowIdx; }
  uint32_t getFirstStoredRowIdx() const { return m_firstStoredIdx; }

  uint32_t getCapacity() const { return m_capacity; }
};

inline std::unique_ptr<RowQueue> RowQueue::s_instance = nullptr;
