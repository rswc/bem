#include <iostream>

#include "handler.h"
#include "messages.h"

void handleResultMessage(State &state, int node_id, ResultMessage *msg) {
    task_id_t task_id = msg->get_task_id();
    Result res = msg->get_result();

    std::cout << "[WT]: Node " << node_id << " sent RESULT for subtask_id = " << task_id << " -> " << res << std::endl;

    state.mtx_tasks.lock();
    state.mtx_groups.lock();

    auto task = state.tasks[task_id];

    if (task->status == TS_RUNNING) {
        auto group = state.groups[task->group_id];
        
        if (group->status == TS_DONE) {
            std::cout << "[WT]: Received result for group task that status is TS_DONE, ignoring." << std::endl;
        } else {
            group->remaining_tasks--;
            group->aggregate_result = group->aggregate_result.merge(res);

            std::cout << "- GROUP [" << group->id << "] now has " << group->remaining_tasks << " remaining." << std::endl;

            if (group->remaining_tasks == 0) {
                group->status = TS_DONE;
                std::cout << "- GROUP [" << group->id << "] has finished processing -> " << group->aggregate_result << std::endl;
            }
            task->status = TS_DONE;
        }
        
    }

    state.mtx_tasks.unlock();
    state.mtx_groups.unlock();
}

void handleHelloMessage(State &state, int node_id, HelloMessage *msg) {
    std::cout << "[WT]: Node " << node_id << " sent HELLO message!" << std::endl;

    state.mtx_nodes.lock();

    if (state.nodeExists(node_id)) {
        state.nodes[node_id]->mark_registered();
        std::cout << "[WT]: Set node " << node_id << " as REGISTERED. Sending Hello Response" << std::endl;
        state.nodes[node_id]->gamelist = msg->gamelist(); 
        // state.nodes[node_id]->gamelist.print(); std::cout.flush();

        GameList empty_gl;
        auto hello_msg = std::make_unique<HelloMessage>();
        
        HelloMessage::HelloFlag flag = HelloMessage::HelloFlag::ACCEPT;
        
        // Make sure to terminate the connection in good way
        state.mtx_config.lock();
        if (msg->protocol_version() != state.config.protocol_version) {
            flag = HelloMessage::HelloFlag::REJECT;
        }
        state.mtx_config.unlock();

        // For now accept every incoming valid message
        hello_msg->init(state.config.protocol_version, flag, empty_gl);
        state.nodes[node_id]->Send(std::move(hello_msg));

    } else {
        std::cout << "[WT]: Node with ID " << node_id << " does not exist." << std::endl;
    }
    state.mtx_nodes.unlock();
}

void handleNotifyTaskMessage(State &state, int node_id, TaskNotifyMessage *msg) {
    // maybe assert before handling that node exists?
    state.mtx_nodes.lock();
    state.mtx_tasks.lock();

    if (state.nodeExists(node_id) && state.nodes[node_id]->is_registered()) {
        auto node = state.nodes[node_id];
        
        node->mark_response();

        if (msg->task_status == TS_RUNNING) {
            node->activeTaskGroup = state.tasks[msg->task_id]->group_id;

        } else {
            node->activeTaskGroup = TASK_ID_NONE;

        }
    }

    state.mtx_nodes.unlock();
    state.mtx_tasks.unlock();
}

void handleCoordinatorMessages(State &state) {
    while (!state.shouldQuit) {

        std::unique_lock<std::mutex> guard(state.mtx_recvQueue);
        state.cv_recvQueue.wait(guard, [&]() { 
            return !state.recvMessageQueue.empty(); 
        });

        auto[node_id, msg] = std::move(state.recvMessageQueue.front());
        state.recvMessageQueue.pop_front();
        guard.unlock();

        switch (msg->GetType()) {
            case BaseMessage::RESULT: {
                ResultMessage *msg_ptr = dynamic_cast<ResultMessage*>(msg.get());
                handleResultMessage(state, node_id, msg_ptr); 
            } break;
            case BaseMessage::HELLO: {
                HelloMessage *msg_ptr = dynamic_cast<HelloMessage*>(msg.get());
                handleHelloMessage(state, node_id, msg_ptr); 
            } break;
            case BaseMessage::TASK_NOTIFY: {
                TaskNotifyMessage *msg_ptr = dynamic_cast<TaskNotifyMessage*>(msg.get());
                handleNotifyTaskMessage(state, node_id, msg_ptr);
            } break;
            case BaseMessage::NONE: {
                state.terminateNode(node_id);
            } break;
            default: {
                assert(0 && "Handler not implemented!");
            }
        }
    }
}