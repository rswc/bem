#include "execute.h"
#include "graceful.h"
#include "messages.h"
#include "protocol.h"
#include "state.h"

#include <thread> // this_thread::sleep_for
#include <memory> // make_unique
#include <chrono> // std::chrono::seconds
#include <iostream>

#define GAME_OUTPUT_BUFFER_SIZE 16

// GameOutcomeFlags
enum {
    GOF_NONE = 0,
    GOF_GAME_EXCEPTION = 1 << 0,
    GOF_PLAYER1_WINNER = 1 << 1,
    GOF_PLAYER2_WINNER = 1 << 2,
    GOF_REASON_TIMEOUT = 1 << 3
};

int launch_subprocess(const std::string& command);
std::string prepare_command(const std::string& games_dir, const std::string& game_name, const std::string& game_jar, 
    const std::string& agent1_jar, const std::string& agent2_jar,  
    uint32_t move_limit_ms, uint32_t board_size);

void execute_tasks_in_loop() {
    while (!getGlobalState().should_quit) {
        std::unique_lock<std::mutex> guard(getGlobalState().mtx_taskQueue);
        getGlobalState().cv_taskQueue.wait(guard, [&]{
            return getGlobalState().should_quit || !getGlobalState().taskQueue.empty();
        });
        
        if (getGlobalState().should_quit) break;

        Task task = getGlobalState().taskQueue.front();
        getGlobalState().taskQueue.pop_front();
        guard.unlock();
        
        // only this line can modify current id from NONE
        getGlobalState().current_task_id = task.id;
        Result result = execute_task(task);

        // break from the game, dont sent incorrect results
        if (getGlobalState().should_quit) break;

        if (getGlobalState().current_task_id == TASK_ID_NONE) {
            std::cout << "[ET]: Task with id " << task.id << " CANCELED. Ignoring result" << std::endl; 
        } else {
            std::cout << "[ET]: Task with id " << task.id << ", DONE. Sending result." << std::endl;
            auto result_msg = std::make_unique<ResultMessage>();
            result_msg->init(task.id, result);
            send_message(std::move(result_msg));
            getGlobalState().current_task_id = TASK_ID_NONE;
        }
    }
    std::cerr << "[ET]: Execute tasks loop broken. Returning" << std::endl;
    terminate_program();
}

std::string prepare_command(const std::string& games_dir, const std::string& game_launcher, 
    const std::string& game_name, const std::string& agent1_path, const std::string& agent2_path,  
    uint32_t move_limit_ms, uint32_t board_size) {

    std::stringstream ss;
    ss << " " << games_dir << "/" << game_launcher;
    ss << " " << games_dir << "/" << agent1_path;
    ss << " " << games_dir << "/" << agent2_path;
    ss << " " << game_name;
    ss << " " << board_size;
    ss << " " << move_limit_ms;
    // ignore stderr
    // ss << " 2> /dev/null";

    return ss.str();
}

Result execute_task(const Task& task) {
    
    getGlobalState().mtx_config.lock();
    std::string game_launcher = getGlobalState().config.game_launcher;
    std::string games_dir = getGlobalState().config.games_dir;
    std::string game_name = getGlobalState().config.gamelist.get_game_name(task.game_id);
    std::string ag1_path = getGlobalState().config.gamelist.get_agent_relative_path(task.game_id, task.agent1);
    std::string ag2_path = getGlobalState().config.gamelist.get_agent_relative_path(task.game_id, task.agent2);
    getGlobalState().mtx_config.unlock();
    
    std::string game_command = prepare_command(games_dir, game_launcher, game_name, ag1_path, ag2_path, task.move_limit_ms, task.board_size);
    

    Result result;
    for (uint32_t i = 0u; i < task.games; i++) {
        if (getGlobalState().should_quit || getGlobalState().current_task_id == TASK_ID_NONE) 
            break;

        std::cout << "> [" << task.id << "] (" << i + 1 << "/" << task.games <<") Running: " << game_command << std::endl;


        int flags = launch_subprocess(game_command);
        result.games++;

        if (flags & GOF_GAME_EXCEPTION) {
            result.failed_games++;
            continue;
        }

        if (flags & GOF_PLAYER1_WINNER) {
            result.win_agent1++;
            if (flags & GOF_REASON_TIMEOUT) {
                result.timeout_agent2++;
            }
        }
        if (flags & GOF_PLAYER2_WINNER) {
            result.win_agent2++;
            if (flags & GOF_REASON_TIMEOUT) {
                result.timeout_agent1++;
            }
        }
    }

    return result;
}

int launch_subprocess(const std::string& command) {
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return GOF_GAME_EXCEPTION;
    
    char buffer[GAME_OUTPUT_BUFFER_SIZE];
    std::string result = "";
    while(!feof(pipe) && !getGlobalState().should_quit && getGlobalState().current_task_id != TASK_ID_NONE) {
    	if(fgets(buffer, GAME_OUTPUT_BUFFER_SIZE, pipe) != NULL)
    		result += buffer;
    }
    int status = pclose(pipe);
    if (WEXITSTATUS(status) != 0) {
        return GOF_GAME_EXCEPTION;
    }

    std::stringstream ss(result);
    std::cout << "[EJ]: " << ss.str() << std::endl;
    
    std::string agent1_name;
    std::string agent2_name;
    std::string win_prompt;
    std::string reason;
    
    std::getline(ss, agent1_name, ';');
    std::getline(ss, agent2_name, ';');
    std::getline(ss, win_prompt, ';');
    std::getline(ss, reason, ';');

    int game_outcome = GOF_NONE;
    if (reason == "TIMEOUT") {
        game_outcome |= GOF_REASON_TIMEOUT;
    }

    if (win_prompt == "\"PLAYER1\"") {
        game_outcome |= GOF_PLAYER1_WINNER;
    } else if (win_prompt == "\"PLAYER2\"") {
        game_outcome |= GOF_PLAYER2_WINNER;
    } 

    return game_outcome;
}
