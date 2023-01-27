#include <vector>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <string>
#include <sstream>
#include <mutex>
#include <cassert>

#include "node.h"
#include "accept.h"
#include "state.h"
#include "taskMessage.h"
#include "handler.h"
#include "maintenance.h"

#include "messages.h"

#include "gamelist.h"
#include "json.hpp"
#include "config.h"
#include "commands.h"


bool setup(State& state, const std::string& configpath) {
    return load_config_from_file(state.config, configpath);
}

int main (int argc, char* argv[]) {
    std::string configpath = std::string(DEFAULT_CONFIG_FILENAME);
    if (argc == 2) {
        configpath = std::string(argv[1]);
    } 
    
    State state;
    bool result = setup(state, configpath);
    
    if (!result) {
        std::cout << "[!] Cannot load config from " << configpath << std::endl;
        return 1;
    }

    std::thread t_acc(acceptConnections, state.config.port, std::ref(state));
    std::thread t_handler(handleCoordinatorMessages, std::ref(state));
    std::thread t_maintenance(doMaintenance, std::ref(state));

    std::cout << "= Bot Evaluation Machine v1.0\n= \n=  Iusiurandum, patri datum, usque\n"
            << "=  ad hanc diem ita servavi\n= \n= Type 'help' to view available commands." << std::endl;
    
    // Temporary scuff interface
    std::string cmd, token;
    while (!state.shouldQuit) // placeholder: replace with epoll in threads?
    {
        std::cout << "> ";
        std::cin >> cmd;

        if (cmd == "help" || cmd == "?") command_help();
        else if (cmd == "terminate") command_terminate(state);
        else if (cmd == "task") command_task(state);
        else if (cmd == "nodes") command_nodes(state);
        else if (cmd == "tasks") command_tasks(state);
        else if (cmd == "cancel") command_cancel(state);
        else if (cmd == "games") command_games(state);
        else if (cmd == "quit") { state.shouldQuit = true; }
        else
        {
            std::cout << "Invalid command. \n";
        }
    }

    t_acc.join();
    t_handler.join();
    t_maintenance.join();
    // join all from state.threads

    return 0;
}
