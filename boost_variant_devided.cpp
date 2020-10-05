#pragma once

#include <boost/variant.hpp>
#include "constrained_objects.h"

namespace {

template <typename...>
struct is_one_of {
  static constexpr bool value = false;
};

template <typename F, typename S, typename... Rest>
struct is_one_of<F, S, Rest...> {
  static constexpr bool value =
      std::is_same<F, S>::value || is_one_of<F, Rest...>::value;
};

}  // namespace

namespace flightkeys {

class Constrained_object {
#define LIST_1                                                                 \
  Satisfied, Point_to_close, Airway_to_close, Airspace_to_close, Sid_to_close, \
      Star_to_close, Unsatisfiable, Force_to_point, Force_to_airway,           \
      Force_to_airspace, Force_to_sid, Force_to_star,                          \
      Terminal_procedure_to_close, Force_to_terminal_procedure,                \
      Segment_to_close, Force_to_segment

#define LIST_2                                                                 \
  Close_point_for_cruise, Close_airway_for_cruise, Close_airspace_for_cruise,  \
      Close_sid_for_cruise, Close_star_for_cruise,                             \
      Close_terminal_procedure_for_cruise, Close_segment_for_cruise,           \
      Force_point_to_cruise, Force_airway_to_cruise, Force_airspace_to_cruise, \
      Force_sid_to_cruise, Force_star_to_cruise,                               \
      Force_terminal_procedure_to_cruise, Force_segment_to_cruise

  using Impl_1 = boost::variant<LIST_1>;
  using Impl_2 = boost::variant<LIST_2>;

  template <class Type, typename... Types>
  using enable_if_one_of = std::enable_if_t<is_one_of<Type, Types...>::value>;

 public:
  /** ctor for types from LIST_1 */
  template <class T1, class = enable_if_one_of<std::decay_t<T1>, LIST_1>>
  Constrained_object(T1&& impl_1, int fake_int = 0)
      : impl_1_(std::forward<T1>(impl_1)) {
    ASSERT_LOGIC(impl_1_ && !impl_2_);
  }

  /** ctor for types from LIST_2 */
  template <class T2, class = enable_if_one_of<std::decay_t<T2>, LIST_2>>
  Constrained_object(T2&& impl_2, bool fake_bool = false)
      : impl_2_(std::forward<T2>(impl_2)) {
    ASSERT_LOGIC(!impl_1_ && impl_2_);
  }

  Constrained_object(Constrained_object const&) = default;
  Constrained_object(Constrained_object&&) = default;
  Constrained_object& operator=(Constrained_object const&) = default;
  Constrained_object& operator=(Constrained_object&&) = default;

  Constrained_object invert() const {
    if (impl_1_)
      return boost::apply_visitor(
          [](auto& impl) { return Constrained_object{impl.invert()}; },
          *impl_1_);

    if (impl_2_)
      return boost::apply_visitor(
          [](auto& impl) { return Constrained_object{impl.invert()}; },
          *impl_2_);

    ASSERT_UNREACHABLE();
  }

  template <class U, class = enable_if_one_of<std::decay_t<U>, LIST_1>>
  U const* get(bool fake_bool = false) const {
    ASSERT_LOGIC(!!impl_1_ ^ !!impl_2_);
    if (impl_1_) return boost::get<U>(&impl_1_.get());
    return nullptr;
  }

  template <class U, class = enable_if_one_of<std::decay_t<U>, LIST_2>>
  U const* get(int fake_int = 0) const {
    ASSERT_LOGIC(!!impl_1_ ^ !!impl_2_);
    if (impl_2_) return boost::get<U>(&impl_2_.get());
    return nullptr;
  }

  bool operator==(Constrained_object const& rhs) const = delete;

  bool operator<(Constrained_object const& rhs) const {
    if (rhs.impl_1_ && impl_1_) {
      return *rhs.impl_1_ < *impl_1_;
    }

    if (rhs.impl_2_ && impl_2_) {
      return *rhs.impl_2_ < *impl_2_;
    }

    return false;
  }

  friend std::ostream& operator<<(std::ostream& os,
                                  Constrained_object const& c);

 private:
  boost::optional<Impl_1> impl_1_;
  boost::optional<Impl_2> impl_2_;
};

inline std::ostream& operator<<(std::ostream& os, Constrained_object const& c) {
  if (c.impl_1_) return os << *c.impl_1_;
  if (c.impl_2_) return os << *c.impl_2_;
  ASSERT_UNREACHABLE();
}

}  // namespace flightkeys


