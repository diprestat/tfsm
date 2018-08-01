#ifndef STRUCTS_H
#define STRUCTS_H
#include <vector>
#include <utility>
#include <limits>
#include "machine/tfsm_to.h"
#include "machine/tfsm.h"
typedef std::pair<std::string, int> ts;
typedef std::vector<ts> sequence;
typedef std::vector<int> path;
const int inf = std::numeric_limits<double>::infinity();
#endif // STRUCTS_H