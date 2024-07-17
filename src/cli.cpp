#include "cli.hpp"

template <>
auto option_with_argument<std::string>(const char *name,
                                       const char *desc) -> cli_option_t {
  return cli_option_t{
      name, desc, true, [](const char *o) -> std::string { return {o}; }};
}

template <>
auto option_with_argument<double>(const char *name,
                                  const char *desc) -> cli_option_t {
  return cli_option_t{
      name, desc, true, [](const char *o) -> double { return std::stod(o); }};
}

template <>
auto option_with_argument<size_t>(const char *name,
                                  const char *desc) -> cli_option_t {
  return cli_option_t{
      name, desc, true, [](const char *o) -> size_t { return std::stoull(o); }};
}

template <>
auto option_with_argument<unsigned long long>(
    const char *name, const char *desc) -> cli_option_t {
  return cli_option_t{
      name, desc, true, [](const char *o) -> size_t { return std::stoull(o); }};
}

template <>
auto option_with_argument<bool>(const char *name,
                                const char *desc) -> cli_option_t {
  return cli_option_t{name, desc, true, [](const char *o) -> bool {
                        std::string arg(o);
                        std::transform(arg.begin(),
                                       arg.end(),
                                       arg.begin(),
                                       [](char c) { return tolower(c); });
                        if (arg == "off") { return false; }
                        if (arg == "on") { return true; }
                        throw cli_option_argument_not_found{
                            "Argument should be ieither on or off"};
                      }};
}
