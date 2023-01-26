#pragma once
#include "state.h"

void send_message(std::unique_ptr<BaseMessage> message);
std::unique_ptr<BaseMessage> receive_message();
bool register_to_coordinator();
void wait_for_instructions_in_loop();