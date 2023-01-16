#pragma once
#include "state.h"

void send_message(State& state, std::unique_ptr<BaseMessage> message);
std::unique_ptr<BaseMessage> receive_message(State& state);
bool register_to_coordinator(State &state);