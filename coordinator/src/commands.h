#pragma once
#include "state.h"

void command_help();
void command_terminate(State& state); 
void command_tasks(State& state); 
void command_games(State& state); 
void command_cancel(State& state); 
void command_nodes(State& state); 
void command_task(State& state); 