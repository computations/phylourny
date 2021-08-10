#include "debug.h"
#include "single_node.hpp"
#include "tournament.hpp"
#include "tournament_node.hpp"
#include "util.hpp"
#include <algorithm>
#include <bits/stdint-uintn.h>
#include <cstdlib>
#include <stdint.h>

template <> void tournament_t<tournament_node_t>::relabel_indicies() {
  _head->assign_internal_labels();
  _head->relabel_indicies(0);
  _head->set_tip_bitset(tip_count());
}

template <> void tournament_t<single_node_t>::relabel_indicies() {
  _head->assign_internal_labels();
  _head->relabel_indicies(0);
  _head->set_tip_bitset(tip_count());
  dynamic_cast<single_node_t &>(*_head).init_assigned_teams();
}
