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

    // no mutex needed, we dont assume multihtreading here yet
    bool setup_ok = load_node_config_from_file(getGlobalState().config, configpath);
    if (!setup_ok) {
        std::cerr << "[!] Reading configuration file resulted in failure. Make sure JSON config is valid." << std::endl;
        return 1;
    }
    
    bool gamelist_valid = verify_loaded_gamelist(getGlobalState().config.gamelist, getGlobalState().config.games_dir);
    if (!gamelist_valid) {
        std::cerr << "[!] Validating GameList resulted in failure. Make sure all JAR files are present." << std::endl;
        return 1;
    }

    // set getGlobalState().socket
    bool connection_established = connect_to_server();
    if (!connection_established) {
        std::cerr << "[!] Connection to coordinator could not be establised. Make sure server is up and running." << std::endl;
        return 1;
    }

    int sock = getGlobalState().socket;
    std::thread t_write(write_to_server_in_loop, sock);
    std::thread t_read(read_from_server_in_loop, sock, 32);
    std::thread t_task(execute_tasks_in_loop);

    bool register_ok = register_to_coordinator();
    if (!register_ok) {
        std::cerr << "[!] Hello-Handshake with coordinator was not completed." << std::endl;
        terminate_program();
    }

    wait_for_instructions_in_loop();
    t_write.join();
    t_read.join();
    t_task.join();

    return 0;
}

