#include <algorithm>
#include <chrono>
#include <iostream>
#include <json.hpp>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>

#include "cli.hpp"
#include "debug.h"
#include "tournament.hpp"
int __VERBOSE__ = EMIT_LEVEL_PROGRESS;

#define STRING(s) #s
#define STRINGIFY(s) STRING(s)
#define GIT_REV_STRING STRINGIFY(GIT_REV)
#define GIT_COMMIT_STRING STRINGIFY(GIT_COMMIT)
#define BUILD_DATE_STRING STRINGIFY(BUILD_DATE)

typedef std::chrono::time_point<std::chrono::high_resolution_clock> timepoint_t;
typedef std::chrono::duration<double> duration_t;

void print_version() {
  debug_string(EMIT_LEVEL_IMPORTANT, "Running Phylourny");
  debug_print(EMIT_LEVEL_IMPORTANT, "Version: %s", GIT_REV_STRING);
  debug_print(EMIT_LEVEL_IMPORTANT, "Build Commit: %s", GIT_COMMIT_STRING);
  debug_print(EMIT_LEVEL_IMPORTANT, "Build Date: %s", BUILD_DATE_STRING);
}

void print_end_time(timepoint_t start_time, timepoint_t end_time) {
  duration_t duration = end_time - start_time;
  debug_print(EMIT_LEVEL_IMPORTANT, "Run Finished, time: %fs",
              duration.count());
}

int main(int argc, char **argv) {
  __VERBOSE__ = EMIT_LEVEL_PROGRESS;
  auto start_time = std::chrono::high_resolution_clock::now();
  print_version();
  cli_options_t cli_options{argc, argv};

  std::shared_ptr<tournament_node_t> n1{new tournament_node_t{"a"}};
  std::shared_ptr<tournament_node_t> n2{new tournament_node_t{"b"}};
  std::shared_ptr<tournament_node_t> n3{new tournament_node_t{"c"}};
  std::shared_ptr<tournament_node_t> n4{new tournament_node_t{"d"}};

  std::shared_ptr<tournament_node_t> w1{
      new tournament_node_t{
          (n1),
          tournament_edge_t::edge_type_t::win,
          (n2),
          tournament_edge_t::edge_type_t::win,
      },
  };

  std::shared_ptr<tournament_node_t> w2{
      new tournament_node_t{
          (n3),
          tournament_edge_t::edge_type_t::win,
          (n4),
          tournament_edge_t::edge_type_t::win,
      },
  };

  std::shared_ptr<tournament_node_t> w3{
      new tournament_node_t{
          (w1),
          tournament_edge_t::edge_type_t::win,
          (w2),
          tournament_edge_t::edge_type_t::win,
      },
  };

  std::shared_ptr<tournament_node_t> l3{
      new tournament_node_t{
          (w1),
          tournament_edge_t::edge_type_t::loss,
          (w2),
          tournament_edge_t::edge_type_t::loss,
      },
  };

  std::shared_ptr<tournament_node_t> l4{
      new tournament_node_t{
          w3,
          tournament_edge_t::edge_type_t::loss,
          l3,
          tournament_edge_t::edge_type_t::win,
      },
  };

  tournament_t t{tournament_node_t{
      l4,
      w3,
  }};

  t.relabel_indicies();
  size_t tip_count = t.tip_count();
  debug_print(EMIT_LEVEL_IMPORTANT, "tip_count: %lu", tip_count);
  auto m = uniform_matrix_factory(tip_count);
  t.reset_win_probs(m);

  auto r = t.eval();
  debug_print(EMIT_LEVEL_IMPORTANT, "eval(): %s", to_string(r).c_str());

  auto end_time = std::chrono::high_resolution_clock::now();
  print_end_time(start_time, end_time);
  return 0;
}
