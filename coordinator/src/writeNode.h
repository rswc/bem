#pragma once

#include "node.h"
#include "state.h"

void writeNode(int sock, std::shared_ptr<Node> node, State& state);
