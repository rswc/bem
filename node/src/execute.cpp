#include "execute.h"
#include "graceful.h"
#include "messages.h"
#include "protocol.h"

#include <thread> // this_thread::sleep_for
#include <memory> // make_unique
#include <chrono> // std::chrono::seconds
#include <iostream>

void execute_tasks_in_loop(State &state) {
    while (!state.should_quit) {
        std::unique_lock<std::mutex> guard(state.mtx_taskQueue);
        state.cv_taskQueue.wait(guard, [&]{
            return state.should_quit || !state.taskQueue.empty();
        });
        
        if (state.should_quit) break;

        Task task = state.taskQueue.front();
        state.taskQueue.pop_front();
        guard.unlock();
        
        state.current_task_id = task.id;
        Result result = execute_task(state, task);
        std::cout << "[ET]: Task with id " << task.id << ", done. Sending result." << std::endl;

        auto result_msg = std::make_unique<ResultMessage>();
        result_msg->init(task.id, result);
        send_message(state, std::move(result_msg));
        state.current_task_id = TASK_ID_NONE;
    }
    std::cerr << "[ET]: Execute tasks loop broken. Returning" << std::endl;
    terminate_program(state);
}

std::string prepare_command(const std::string& games_dir, const std::string& game_name, const std::string& game_jar, 
    const std::string& agent1_jar, const std::string& agent2_jar,  
    uint32_t move_limit_ms, uint32_t board_size) {

    std::stringstream ss;
    ss << "java -jar";
    ss << " " << games_dir << "/" << game_jar;
    ss << " " << games_dir << "/" << agent1_jar;
    ss << " " << games_dir << "/" << agent2_jar;
    ss << " " << game_name;
    ss << " " << board_size;
    ss << " " << move_limit_ms;
    // ignore stderr
    ss << " 2> /dev/null";

    return ss.str();
}

Result execute_task(State& state, const Task& task) {
    
    state.mtx_config.lock();
    std::string games_dir = state.config.games_dir;
    std::string game_name = state.config.gamelist.get_game_name(task.game_id);
    std::string game_jar = state.config.gamelist.get_game_relative_jar_path(task.game_id);
    std::string ag1_jar = state.config.gamelist.get_agent_relative_jar_path(task.game_id, task.agent1);
    std::string ag2_jar = state.config.gamelist.get_agent_relative_jar_path(task.game_id, task.agent2);
    state.mtx_config.unlock();
    
    std::string game_command = prepare_command(games_dir, game_name, game_jar, ag1_jar, ag2_jar, task.move_limit_ms, task.board_size);
    
    std::cout << "[ET]: running: " << game_command << std::endl;
    Result result;
    result.games = 0;
    result.win_agent1 = 0;
    result.win_agent2 = 0;
    
    for (uint32_t i = 0u; i < task.games; i++) {
        int winner = 0; // 1 if first, 2 if second, 0 if draft
        bool exited_safely = launch_subprocess(state, game_command, winner);
        if (!exited_safely) continue;
        
        result.games++;
        if (winner & 1) result.win_agent1++;
        if (winner & 2) result.win_agent2++;
    }

    return result;
}

bool launch_subprocess(State& state, const std::string& command, int& winner) {
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return false;
    
    char buffer[128];
    std::string result = "";
    while(!feof(pipe) && !state.should_quit) {
    	if(fgets(buffer, 128, pipe) != NULL)
    		result += buffer;
    }

    pclose(pipe);
    std::cout << "[EJ]: Popen printed " << result.size() << " characters." << std::endl;
    
    std::stringstream ss(result);
    std::cout << "[EJ]: " << ss.str() << std::endl;
    
    std::string agent1_name;
    std::string agent2_name;
    std::string win_prompt;
    
    std::getline(ss, agent1_name, ';');
    std::getline(ss, agent2_name, ';');
    std::getline(ss, win_prompt, ';');

    std::cout << "agent1: " << agent1_name << std::endl;
    std::cout << "agent2: " << agent2_name << std::endl;
    std::cout << "win_prompt: " << win_prompt << std::endl;

    if (win_prompt == "\"PLAYER1\"") {
        winner = 1;
    } else if (win_prompt == "\"PLAYER2\"") {
        winner = 2;
    } else {
        winner = 0;
    }

    return true;
}
