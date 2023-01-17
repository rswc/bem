#include "graceful.h"

#include <unistd.h>
#include <sys/socket.h>
#include <csignal>

void terminate_program(State& state) {

    // change to condition variable, to notify all threads that terminating is done
    state.mtx_terminate.lock();
    if (!state.should_quit) {
        // pass information to others?
        state.should_quit = true; 

        // receive_message -> wait_for_instructions_in_loop
        state.cv_recvQueue.notify_all();
        
        // execute_tasks_in_loop
        state.cv_taskQueue.notify_all();
        
        // write_to_server_in_loop
        state.cv_sendQueue.notify_all();

        // read_from_server_in_loop, write_to_server_in_loop
        // TODO: SHUT_RW or SHUT_RWRD?
        shutdown(state.socket, SHUT_RD);
    }
    state.mtx_terminate.unlock();
}


void sigint_handler(int signal) {}

void init_sigint_handler() {
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = sigint_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, nullptr);
}