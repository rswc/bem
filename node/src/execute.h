#pragma once
#include "state.h"
#include "result.h"
#include "task.h"

void execute_tasks_in_loop();
Result execute_task(const Task& task);