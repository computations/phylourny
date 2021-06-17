#ifndef __CLIHPP__
#define __CLIHPP__

#include "debug.h"
#include <any>
#include <cstring>
#include <exception>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

class cli_option_exception : public std::exception {};

class cli_option_not_recognized : public cli_option_exception {
public:
  cli_option_not_recognized(std::string m) : _what{m} {}
  const char *what() const noexcept override { return _what.c_str(); }

private:
  std::string _what;
};

class cli_option_argument_not_found : public cli_option_exception {
public:
  cli_option_argument_not_found(std::string m) : _what{m} {}
  const char *what() const noexcept override { return _what.c_str(); }

private:
  std::string _what;
};

class cli_option_not_initialized : public cli_option_exception {
public:
  cli_option_not_initialized(std::string m) : _what{m} {}
  const char *what() const noexcept override { return _what.c_str(); }

private:
  std::string _what;
};

class cli_option_help : public cli_option_exception {
public:
  const char *what() const noexcept override {
    return "Found help on the command line";
  }
};

class cli_option_t {
public:
  cli_option_t(const char *name, const char *desc, bool argument,
               std::function<std::any(const char *)> parser)
      : _name{name}, _description{desc}, _argument{argument}, _opt_parser{
                                                                  parser} {}
  cli_option_t(const char *name, const char *desc, bool argument)
      : _name{name}, _description{desc}, _argument{argument} {}

  bool has_argument() const { return _argument; }

  cli_option_t &required() {
    _required = true;
    return *this;
  }

  const char *name() const { return _name; }

  void consume(const char *o) {
    _optarg = o;
    if (_opt_parser.has_value()) {
      _opt_val = (*_opt_parser)(_optarg);
    }
  }

  void flag() { _opt_val = true; }

  bool initialized() const { return _opt_val.has_value(); }

  template <typename T> T value() {
    if (!initialized()) {
      throw cli_option_not_initialized{
          std::string("Did not find a value for ") + _name};
    }
    return std::any_cast<T>(_opt_val);
  }

  template <typename T> T value(const T &default_value) {
    if (!initialized()) {
      return default_value;
    }
    return std::any_cast<T>(_opt_val);
  }

  std::string help(int align = 20) const {
    std::stringstream oss;
    oss << "--" << _name;

    align -= (2 + strlen(_name));
    if (_argument) {
      oss << " <VALUE>";
      align -= 8;
    }

    for (int i = 0; i < align; i++) {
      oss << " ";
    }

    oss << _description;

    if (_required) {
      oss << " (REQUIRED)";
    }

    return oss.str();
  }

private:
  const char *_name;
  const char *_description;
  const char *_optarg;
  bool _required;
  bool _argument;
  std::any _opt_val;
  std::optional<std::function<std::any(const char *)>> _opt_parser;
};

template <typename T>
cli_option_t option_with_argument(const char *name, const char *desc) {
  return cli_option_t{name, desc, true, [](const char *o) -> T { return {o}; }};
}

template <>
cli_option_t option_with_argument<std::string>(const char *name,
                                               const char *desc) {
  return cli_option_t{name, desc, true,
                      [](const char *o) -> std::string { return {o}; }};
}

template <>
cli_option_t option_with_argument<double>(const char *name, const char *desc) {
  return cli_option_t{name, desc, true,
                      [](const char *o) -> double { return std::stod(o); }};
}

template <>
cli_option_t option_with_argument<size_t>(const char *name, const char *desc) {
  return cli_option_t{name, desc, true,
                      [](const char *o) -> size_t { return std::stoull(o); }};
}

cli_option_t option_flag(const char *name, const char *desc) {
  return cli_option_t{
      name,
      desc,
      false,
  };
}

cli_option_t args[] = {
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
    option_flag("dummy", "Make dummy data"),
    option_flag("debug", "Enable debug output"),
};

class cli_options_t {
public:
  cli_options_t() = delete;
  cli_options_t(int argc, char **argv) {
    _option_count = sizeof(args) / sizeof(cli_option_t);
    for (int i = 1; i < argc; ++i) {
      const char *cur_arg = argv[i];
      debug_print(EMIT_LEVEL_DEBUG, "working on argument: %s", cur_arg);
      if (!(cur_arg[0] == '-' && cur_arg[1] == '-')) {
        throw cli_option_not_recognized{std::string{"Failed to recognize "} +
                                        cur_arg};
      }
      cur_arg = cur_arg + 2;
      bool found = false;
      if (strcmp(cur_arg, "help") == 0) {
        std::cout << help();
        throw cli_option_help{};
      }
      for (size_t k = 0; k < _option_count; ++k) {
        if (strcmp(cur_arg, args[k].name()) != 0) {
          continue;
        }
        found = true;
        if (args[k].has_argument()) {
          if (i + 1 >= argc) {
            throw cli_option_argument_not_found{
                std::string{"Did not find an argument to option "} + argv[i]};
          }
          args[k].consume(argv[++i]);
          break;
        } else {
          args[k].flag();
        }
      }
      if (!found) {
        debug_print(EMIT_LEVEL_IMPORTANT,
                    "Failed to recognize command line argument: %s", argv[i]);
      }
    }

    for (size_t i = 0; i < _option_count; ++i) {
      _opt_vals[args[i].name()] = args + i;
    }
  };

  cli_option_t operator[](std::string key) { return *_opt_vals.at(key); }

  static std::string help() {
    std::stringstream oss;
    oss << "Help:\n";
    for (auto a : args) {
      oss << "  " << a.help() << std::endl;
    }
    return oss.str();
  }

private:
  size_t _option_count;
  std::unordered_map<std::string, cli_option_t *> _opt_vals;
};

#endif
