#include <iostream>

#include "handler.h"
#include "messages.h"

void handlePongMessage(State &state, int node_id, PongMessage *msg) {
    std::cout << "[WT]: Node " << node_id << " sent PONG message!";
}

void handleResultMessage(State &state, int node_id, ResultMessage *msg) {
    task_id_t task_id = msg->getId();
    std::cout << "[WT]: Node " << node_id << " sent RESULT for task id: " << task_id << std::endl;
}

void handleHelloMessage(State &state, int node_id, HelloMessage *msg) {
    std::cout << "[WT]: Node " << node_id << " sent HELLO message!" << std::endl;

    
    state.mtx_nodes.lock();

    if (state.nodeExists(node_id)) {
        state.nodes[node_id]->mark_registered();
        std::cout << "[WT]: Set node " << node_id << " as REGISTERED. Sending Hello Response" << std::endl;
        state.nodes[node_id]->gamelist = msg->gamelist(); 
        state.nodes[node_id]->gamelist.print();
        std::cout.flush();

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
    std::cout << "[WT]: Node " << node_id << " sent Notification Message !" << std::endl;

    // maybe assert before handling that node exists?
    state.mtx_nodes.lock();
    if (state.nodeExists(node_id) && state.nodes[node_id]->is_registered()) {
        state.nodes[node_id]->mark_response();
        double reply_time = state.nodes[node_id]->reply_time();
        std::cout << "[WT]: Reply took node [" << node_id << "] " << reply_time << " seconds!" << std::endl;
        if (msg->task_status == TS_RUNNING) {
            std::cout << "Node is running task " << msg->task_id << std::endl;
        } else if (msg->task_status == TS_NONE) {
            std::cout << "Node is currently idle " << std::endl;
        }
    }
    state.mtx_nodes.unlock();
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
            case BaseMessage::NONE: break;
            case BaseMessage::RESULT: {
                ResultMessage *msg_ptr = dynamic_cast<ResultMessage*>(msg.get());
                handleResultMessage(state, node_id, msg_ptr); 
            } break;
            case BaseMessage::HELLO: {
                HelloMessage *msg_ptr = dynamic_cast<HelloMessage*>(msg.get());
                handleHelloMessage(state, node_id, msg_ptr); 
            } break;
            case BaseMessage::PING:  {
                PongMessage *msg_ptr = dynamic_cast<PongMessage*>(msg.get());
                handlePongMessage(state, node_id, msg_ptr);
            } break;
            case BaseMessage::TASK_NOTIFY: {
                TaskNotifyMessage *msg_ptr = dynamic_cast<TaskNotifyMessage*>(msg.get());
                handleNotifyTaskMessage(state, node_id, msg_ptr);
            } break;
            default: {
                assert(0 && "Handler not implemented!");
            }
        }
    }
}