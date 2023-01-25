#include "graceful.h"

#include <unistd.h>
#include <sys/socket.h>
#include <csignal>

void terminate_program() {

    // change to condition variable, to notify all threads that terminating is done
    getGlobalState().mtx_terminate.lock();
    if (!getGlobalState().should_quit) {
        // pass information to others?
        getGlobalState().should_quit = true; 

        // receive_message -> wait_for_instructions_in_loop
        getGlobalState().cv_recvQueue.notify_all();
        
        // execute_tasks_in_loop
        getGlobalState().cv_taskQueue.notify_all();
        
        // write_to_server_in_loop
        getGlobalState().cv_sendQueue.notify_all();

        // read_from_server_in_loop, write_to_server_in_loop
        // TODO: SHUT_RW or SHUT_RWRD?
        shutdown(getGlobalState().socket, SHUT_RD);
    }
    getGlobalState().mtx_terminate.unlock();
}


void sigint_handler(int signum) {
    std::cerr << "[!] Called handler for signal " << signum << "! Terminating Node program" << std::endl;
    terminate_program();
}

void init_sigint_handler() {
    signal(SIGINT, sigint_handler);
}