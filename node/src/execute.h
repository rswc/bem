#pragma once
#include "state.h"
#include "result.h"
#include "task.h"

void execute_tasks_in_loop(State &state);
Result execute_task(State& state, const Task& task);
std::string prepare_command(const std::string& games_dir, const std::string& game_name, const std::string& game_jar, 
    const std::string& agent1_jar, const std::string& agent2_jar,  
    uint32_t move_limit_ms, uint32_t board_size);
bool launch_subprocess(State& state, const std::string& command, int& winner);