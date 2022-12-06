#pragma once

#include "node.h"
#include "state.h"

void readNode(int sock, std::shared_ptr<Node> node, State& state);
