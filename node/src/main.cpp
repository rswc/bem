#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <error.h>
#include <netdb.h>
#include <thread>
#include <iostream>
#include <chrono>

#include "messages.h"

#include "state.h"
#include "config.h"
#include "protocol.h"
#include "communication.h"
#include "execute.h"
#include "graceful.h"


void doTasks(State &state) {
    while (true) {
        std::unique_lock<std::mutex> guard(state.mtx_taskQueue);
        state.cv_taskQueue.wait(guard, [&]{
            return !state.taskQueue.empty();
        });
        Task task = state.taskQueue.front();
        state.taskQueue.pop_front();
        guard.unlock();
        
        state.current_task_id = task.id;
        std::cout << "[DT]: Task with id " << task.id << ", received. Sleeping for 10 seconds" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::cout << "[DT]: Task with id " << task.id << ", done. Sending result" << std::endl;
        
        auto result_msg = std::make_unique<ResultMessage>();
        result_msg->init(task.id);
        send_message(state, std::move(result_msg));
        state.current_task_id = TASK_ID_NONE;
    }
}

int main(int argc, char const *argv[]) {
    std::string configpath = std::string(DEFAULT_CONFIG_FILENAME);
    if (argc == 2) {
        configpath = std::string(argv[1]);
    } 

    State state;

    // no mutex needed, we dont assume multihtreading here yet
    bool setup_ok = load_node_config_from_file(state.config, configpath);
    if (!setup_ok) {
        std::cerr << "[!] Reading configuration file resulted in failure. Make sure JSON config is valid." << std::endl;
        return 1;
    }

    // set socket as state variable
    bool connection_established = connect_to_server(state);
    if (!connection_established) {
        std::cerr << "[!] Connection to coordinator could not be establised. Make sure server is up and running." << std::endl;
        return 1;
    }

    // graceful exit?
    int sock = state.socket;
    std::thread t_write(write_to_server_in_loop, std::ref(state), sock);
    std::thread t_read(read_from_server_in_loop, std::ref(state), sock, 32);
    std::thread t_task(execute_tasks_in_loop, std::ref(state));

    bool register_ok = register_to_coordinator(state);
    if (!register_ok) {
        std::cerr << "[!] Hello-Handshake with coordinator was not completed." << std::endl;
        terminate_program(state);
    }

    wait_for_instructions_in_loop(state);
    t_write.join();
    t_read.join();
    t_task.join();

    return 0;
}

