/**
 * CLI option parser. Header only and templated. Currently is not in a great
 * state, as the library is very unergonomic to consume.
 */
#ifndef CLI_HPP
#define CLI_HPP

#include "debug.h"
#include <any>
#include <cctype>
#include <cstring>
#include <exception>
#include <functional>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>

/** @defgroup cli_errors CLI Parser Error Types
 */

/**
 * Base exception type for cli option parsing
 *
 * @ingroup: cli_errors
 */
class cli_option_exception : public std::exception {};

/**
 * Exception for an option not being recognized by the parser
 *
 * @ingroup: cli_errors
 */
class cli_option_not_recognized : public cli_option_exception {
public:
  explicit cli_option_not_recognized(std::string m) : _what{std::move(m)} {}
  [[nodiscard]] auto what() const noexcept -> const char * override {
    return _what.c_str();
  }

private:
  std::string _what;
};

/**
 * Exception for a required argument not found
 *
 * @ingroup: cli_errors
 */
class cli_option_argument_not_found : public cli_option_exception {
public:
  explicit cli_option_argument_not_found(std::string m) : _what{std::move(m)} {}
  [[nodiscard]] auto what() const noexcept -> const char * override {
    return _what.c_str();
  }

private:
  std::string _what;
};

/**
 * Exception for an arguement being accessed, but not initialized.
 *
 * @ingroup: cli_errors
 */
class cli_option_not_initialized : public cli_option_exception {
public:
  explicit cli_option_not_initialized(std::string m) : _what{std::move(m)} {}
  [[nodiscard]] auto what() const noexcept -> const char * override {
    return _what.c_str();
  }

private:
  std::string _what;
};

/**
 * Exception that signals to print the help
 *
 * @ingroup: cli_errors
 */
class cli_option_help : public cli_option_exception {
public:
  [[nodiscard]] auto what() const noexcept -> const char * override {
    return "Found help on the command line";
  }
};

/**
 * Class that contains a CLI option and its value.
 */
class cli_option_t {
public:
  cli_option_t(const char                           *name,
               const char                           *desc,
               bool                                  argument,
               std::function<std::any(const char *)> parser) :
      _name{name},
      _description{desc},
      _optarg{nullptr},
      _required{false},
      _argument{argument},
      _opt_parser{parser} {}

  cli_option_t(const char *name, const char *desc, bool argument) :
      _name{name},
      _description{desc},
      _optarg{nullptr},
      _required{false},
      _argument{argument} {}

  [[nodiscard]] auto has_argument() const -> bool { return _argument; }

  auto required() -> cli_option_t & {
    _required = true;
    return *this;
  }

  [[nodiscard]] auto name() const -> const char * { return _name; }

  void consume(const char *o) {
    _optarg = o;
    if (_opt_parser.has_value()) { _opt_val = (*_opt_parser)(_optarg); }
  }

  void set_flag() { _opt_val = true; }

  [[nodiscard]] auto initialized() const -> bool {
    return _opt_val.has_value();
  }

  /**
   * Gets the value from a CLI option. Templated because the type of the
   * argument isn't known at runtime (in this version).
   */
  template <typename T> auto value() -> T {
    if (!initialized()) {
      throw cli_option_not_initialized{
          std::string("Did not find a value for ") + _name};
    }
    return std::any_cast<T>(_opt_val);
  }

  /**
   * Gets the value from a CLI option, but with a default value. Templated
   * because the type of the value is not known at runtime.
   */
  template <typename T> auto value(const T &default_value) -> T {
    if (!initialized()) { return default_value; }
    return std::any_cast<T>(_opt_val);
  }

  /**
   * Return help for this option as a string.
   *
   * @param align Controls the gap between the options and help text. Behaves
   * like a "tab stop".
   */
  [[nodiscard]] auto help(size_t align = 20) const -> std::string {
    std::stringstream oss;
    oss << "--" << _name;

    align -= (2 + strlen(_name));
    if (_argument) {
      oss << " <VALUE>";
      align -= _align_offset;
    }

    for (size_t i = 0; i < align; i++) { oss << " "; }

    oss << _description;

    if (_required) { oss << " (REQUIRED)"; }

    return oss.str();
  }

  template <typename T> inline void set_if_unset(const T &new_value) {
    if (!initialized()) { _opt_val = new_value; }
  }

private:
  /**
   * Statically allocated name of the option. Also the text that is parsed on
   * the command line.
   */
  const char *_name;

  /**
   * Description of the option. Used in the help text.
   */
  const char *_description;

  /**
   * Pointer to the actual option argument. Will be used to parse _opt_val
   */
  const char *_optarg;

  bool     _required;
  bool     _argument;
  std::any _opt_val;

  /**
   * Optional custom parser for the value.
   */
  std::optional<std::function<std::any(const char *)>> _opt_parser;

  static constexpr size_t _align_offset = 8;
};

template <typename T>
auto option_with_argument(const char *name, const char *desc) -> cli_option_t {
  return cli_option_t{name, desc, true, [](const char *o) -> T { return {o}; }};
}

template <>
auto option_with_argument<std::string>(const char *name,
                                       const char *desc) -> cli_option_t;

template <>
auto option_with_argument<double>(const char *name,
                                  const char *desc) -> cli_option_t;

template <>
auto option_with_argument<size_t>(const char *name,
                                  const char *desc) -> cli_option_t;

template <>
auto option_with_argument<unsigned long long>(const char *name,
                                              const char *desc) -> cli_option_t;

template <>
auto option_with_argument<bool>(const char *name,
                                const char *desc) -> cli_option_t;

static auto option_flag(const char *name, const char *desc) -> cli_option_t {
  return cli_option_t{
      name,
      desc,
      false,
  };
}

/**
 * Actual CLI options for the program.
 */
static cli_option_t args[] = {
    option_with_argument<std::string>("teams", "File with the team names")
        .required(),
    option_with_argument<std::string>("prefix", "Output files prefix")
        .required(),
    option_with_argument<uint64_t>("seed", "Random engine seed"),
    option_with_argument<std::string>("matches", "Match history as a csv file"),
    option_with_argument<std::string>("odds",
                                      "Odds of teams winning as a csv file"),
    option_with_argument<std::string>(
        "probs", "Pairwise win probabilities as a csv file"),
    option_flag("single", "Compute the tournament in single mode."),
    option_flag("sim", "Compute the tournament in simulation mode."),
    option_flag("dynamic", "Enable or disable dynamic computation"),
    option_with_argument<size_t>("sim-iters",
                                 "Number of simulation iterations to run"),
    option_with_argument<size_t>(
        "samples", "Number of samples to take for the MCMC exploration"),
    option_with_argument<double>(
        "burnin", "Proportion of samples to discard for MCMC burnin"),
    option_with_argument<bool>(
        "poisson", "Use a Poisson based liklihood model for the MCMC search"),
    option_with_argument<std::string>(
        "bestofs",
        "Set the number of best ofs for the tournament. Given as a comma "
        "separated list."),
    option_flag(
        "node-probs",
        "Record node probabilities in addition to tournament probabilities"),
    option_flag("sample-matrix", "Sample the matrix during the MCMC search"),
    option_flag("dummy", "Make dummy data"),
    option_flag("verbose", "Enable more output"),
    option_flag("debug", "Enable debug output"),
};

/**
 * A list of all the options. Also includes a constructor that will parse the
 * options. The default constructor has been deleted.
 */
class cli_options_t {
public:
  cli_options_t() = delete;
  cli_options_t(int argc, char **argv) :
      _option_count(sizeof(args) / sizeof(cli_option_t)) {

    for (int i = 1; i < argc; ++i) {
      const char *cur_arg = argv[i];
      debug_print(EMIT_LEVEL_DEBUG, "working on argument: %s", cur_arg);
      if (!(cur_arg[0] == '-' && cur_arg[1] == '-')) {
        throw cli_option_not_recognized{std::string{"Failed to recognize "} +
                                        cur_arg};
      }
      cur_arg    = cur_arg + 2;
      bool found = false;
      if (strcmp(cur_arg, "help") == 0) { throw cli_option_help{}; }
      for (size_t k = 0; k < _option_count; ++k) {
        if (strcmp(cur_arg, args[k].name()) != 0) { continue; }
        found = true;
        if (args[k].has_argument()) {
          if (i + 1 >= argc) {
            throw cli_option_argument_not_found{
                std::string{"Did not find an argument to option "} + argv[i]};
          }
          args[k].consume(argv[++i]);
          break;
        }
        args[k].set_flag();
      }
      if (!found) {
        debug_print(EMIT_LEVEL_IMPORTANT,
                    "Failed to recognize command line argument: %s",
                    argv[i]);
      }
    }

    for (size_t i = 0; i < _option_count; ++i) {
      _opt_vals[args[i].name()] = args + i;
    }
  }

  /**
   * Use this function to access the parsed CLI options.
   *
   * @param key The CLI option, as a string. No preceding characters (such as
   * '--').
   */
  auto operator[](const std::string &key) -> cli_option_t & {
    return *_opt_vals.at(key);
  }

  auto operator[](const std::string &key) const -> cli_option_t {
    return *_opt_vals.at(key);
  }

  static auto help() -> std::string {
    std::stringstream oss;
    oss << "Help:\n";
    for (const auto &a : args) { oss << "  " << a.help() << std::endl; }
    return oss.str();
  }

private:
  size_t                                          _option_count;
  std::unordered_map<std::string, cli_option_t *> _opt_vals;
};

#endif
