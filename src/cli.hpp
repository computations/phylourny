#ifdef __CLIHPP__
#define __CLIHPP__

class initialized_flag_t {
public:
  enum value_t {
    uninitalized,
    initialized_true,
    initialized_false,
  };
  initialized_flag_t() : value(value_t::uninitalized){};
  constexpr initialized_flag_t(const value_t &v) : value(v) {}
  constexpr bool operator==(const initialized_flag_t &rhs) {
    return rhs.value == value;
  }
  constexpr bool operator!=(const initialized_flag_t &rhs) {
    return rhs.value != value;
  }
  constexpr bool initalized() const { return value != value_t::uninitalized; }
  initialized_flag_t &operator=(const initialized_flag_t &rhs) {
    value = rhs.value;
    return *this;
  }
  bool convert_with_default(bool default_value) {
    if (value == value_t::uninitalized)
      return default_value;
    return value == value_t::initialized_true;
  }

private:
  value_t value;
};

struct cli_options_t {};

#endif
