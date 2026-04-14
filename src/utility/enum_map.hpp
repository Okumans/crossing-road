#pragma once

#include "external/magic_enum.hpp"
#include <algorithm>
#include <array>
#include <format>
#include <initializer_list>
#include <ranges>
#include <stdexcept>
#include <utility>

template <typename Key, typename Value,
          size_t Size = magic_enum::enum_count<Key>()>
class EnumMap {
private:
  std::array<Value, Size> _data;

  static constexpr size_t to_idx(Key key) { return static_cast<size_t>(key); }

  // --- THE FIX ---
  // This helper unpacks the temporary array directly into the std::array
  // during construction. Zero default-initialization, zero mutation.
  template <size_t... Is>
  constexpr EnumMap(Value (&&values)[Size], std::index_sequence<Is...>)
      : _data{std::move(values[Is])...} {}

public:
  constexpr EnumMap() = default;

  // 1. CONSTEXPR CONSTRUCTOR (Implicit Keys)
  // Takes your {{...}, {...}} syntax and delegates to the unpacker
  constexpr EnumMap(Value (&&values)[Size])
      : EnumMap(std::move(values), std::make_index_sequence<Size>{}) {}

  // 2. RUNTIME CONSTRUCTOR (Explicit Keys)
  constexpr EnumMap(std::initializer_list<std::pair<Key, Value>> list)
      : _data{} {
    for (auto &item :
         const_cast<std::initializer_list<std::pair<Key, Value>> &>(list)) {
      _data[to_idx(item.first)] = std::move(item.second);
    }
  }

  // --- Accessors ---
  constexpr Value &at(Key key) { return _data[to_idx(key)]; }
  constexpr const Value &at(Key key) const { return _data[to_idx(key)]; }

  constexpr Value &operator[](Key key) { return _data[to_idx(key)]; }
  constexpr const Value &operator[](Key key) const {
    return _data[to_idx(key)];
  }

  constexpr Value &get_checked(Key key) {
    if (to_idx(key) >= Size) {
      throw std::runtime_error(
          std::format("Key out of bounds: {}", static_cast<int>(key)));
    }
    return _data[to_idx(key)];
  }

  constexpr size_t size() const { return Size; }

  // --- Iterators ---
  constexpr auto begin() noexcept { return _data.begin(); }
  constexpr auto end() noexcept { return _data.end(); }
  constexpr auto begin() const noexcept { return _data.begin(); }
  constexpr auto end() const noexcept { return _data.end(); }

  constexpr auto pairs() const {
    return std::views::iota(size_t(0), Size) |
           std::views::transform([this](size_t i) {
             return std::pair<Key, const Value &>(static_cast<Key>(i),
                                                  _data[i]);
           });
  }
};

template <typename T> struct EnumMapValidator {
  constexpr bool operator()(const T &ref) const {
    return std::ranges::all_of(ref, [](const auto &value) {
      static_assert(std::is_constructible_v<bool, decltype(value)>,
                    "EnumMap elements must be convertible to bool!");

      return static_cast<bool>(value);
    });
  }
};
