#pragma once

#include "state.h"

extern const size_t MAX_NODE_READ_BUFFER_SIZE;

bool connect_to_server(State& state);
void write_to_server_in_loop(State& state, int socket);
void read_from_server_in_loop(State& state, int socket, size_t read_size);