#include "messages.h"
#include "state.h"
#include "config.h"
#include "protocol.h"
#include "communication.h"
#include "execute.h"
#include "graceful.h"

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <error.h>
#include <netdb.h>
#include <thread>
#include <iostream>
#include <chrono>

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
    
    bool gamelist_valid = verify_loaded_gamelist(state.config.gamelist, state.config.games_dir);
    if (!gamelist_valid) {
        std::cerr << "[!] Validating GameList resulted in failure. Make sure all JAR files are present." << std::endl;
        return 1;
    }

    // set state.socket
    bool connection_established = connect_to_server(state);
    if (!connection_established) {
        std::cerr << "[!] Connection to coordinator could not be establised. Make sure server is up and running." << std::endl;
        return 1;
    }

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

