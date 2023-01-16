#include "protocol.h"
#include "state.h"

#include "messages.h"

void send_message(State& state, std::unique_ptr<BaseMessage> message) {
    std::unique_lock<std::mutex> guard(state.mtx_sendQueue);
    state.sendMessageQueue.push_back(std::move(message));
    guard.unlock();
    state.cv_sendQueue.notify_one();
}

std::unique_ptr<BaseMessage> receive_message(State& state) {
    std::unique_lock<std::mutex> guard(state.mtx_recvQueue);
    state.cv_recvQueue.wait(guard, [&]() { 
        return !state.recvMessageQueue.empty(); 
    });

    auto msg = std::move(state.recvMessageQueue.front());
    state.recvMessageQueue.pop_front();
    return msg;
}

bool register_to_coordinator(State &state) {
    std::cout << "Sending HELLO message to coordinator..." << std::endl;
    state.mtx_config.lock();
    auto hello_msg = std::make_unique<HelloMessage>();
    hello_msg->init(state.config.protocol_version, HelloMessage::HelloFlag::NONE, state.config.gamelist);
    send_message(state, std::move(hello_msg));
    state.mtx_config.unlock();

    bool sync_complete = false;
    while (!sync_complete) {
        std::cout << "Waiting for HELLO message from the coordinator..." << std::endl;

        auto msg = receive_message(state);
        if (msg->GetType() != BaseMessage::HELLO) {
            std::cout << "[!] Server returned message but its wrong." << std::endl;
            return false;
        }

        HelloMessage *msg_ptr = dynamic_cast<HelloMessage*>(msg.get());
        switch (msg_ptr->flag()) {
            case HelloMessage::HelloFlag::REJECT: {
                std::cerr << "[!] Server returned REJECT flag in hello response." << std::endl;
                return false;
            } break;
            case HelloMessage::HelloFlag::ACCEPT: {
                std::cerr << "Server returned ACCEPT flag in hello response." << std::endl;
                sync_complete = true;
            } break;
            case HelloMessage::HelloFlag::SYNC: {
                std::cerr << "Server returned SYNC flag in hello response." << std::endl;
                sync_complete = true;
            } break;
            default: {
                std::cerr << "[!] Server returned unknown flag in hello response." << std::endl;
                return false;
            } break;
        }
    }
    return true;
}