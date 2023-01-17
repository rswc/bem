#pragma once
#include "state.h"

void execute_tasks_in_loop(State &state);
bool check_winner(State &state, const Game& game, const Agent& agent1, const Agent& agent2);