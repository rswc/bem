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

        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::cout << "[DT]: Task with id " << task.id << ", done. Sending result" << std::endl;
        
        auto result_msg = std::make_unique<ResultMessage>();
        result_msg->init(task.id);
        send_message(state, std::move(result_msg));
        state.current_task_id = TASK_ID_NONE;
    }
    std::cerr << "[ET]: Execute tasks loop broken. Returning" << std::endl;
    terminate_program(state);
}


bool check_winner(State &state, const Game& game, const Agent& agent1, const Agent& agent2) {
}