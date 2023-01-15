#include <iostream>

#include "handler.h"
#include "messages.h"


void handlePongMessage(State &state, int node_id, PongMessage *msg) {
    std::cout << "[WT]: Node " << node_id << " sent PONG message!";
}

void handleResultMessage(State &state, int node_id, ResultMessage *msg) {
    uint32_t task_id = msg->getId();
    std::cout << "[WT]: Node " << node_id << " sent RESULT for task id: " << task_id << std::endl;
}

void handleHelloMessage(State &state, int node_id, HelloMessage *msg) {
    std::cout << "[WT]: Node " << node_id << " sent HELLO message!" << std::endl;

    msg->gamelist().print();
    std::cout.flush();
    
    state.mtx_nodes.lock();

    if (state.nodeExists(node_id)) {
        state.nodes[node_id]->mark_registered();
        std::cout << "[WT]: Set node " << node_id << " as REGISTERED. Sending Hello Response" << std::endl;
        state.nodes[node_id]->gamelist = msg->gamelist(); 

        GameList empty_gl;
        auto hello_msg = std::make_unique<HelloMessage>();
        // For now accept every incoming valid message
        hello_msg->init(state.config.protocol_version, HelloMessage::HelloFlag::ACCEPT, empty_gl);
        state.nodes[node_id]->Send(std::move(hello_msg));

    } else {
        std::cout << "[WT]: Node with ID " << node_id << " does not exist." << std::endl;
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
            }
            default: {
                
            }
        }
    }
}