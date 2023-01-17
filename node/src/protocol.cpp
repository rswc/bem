#include "protocol.h"
#include "state.h"

#include "messages.h"
#include "graceful.h"

void send_message(State& state, std::unique_ptr<BaseMessage> message) {
    std::unique_lock<std::mutex> guard(state.mtx_sendQueue);
    state.sendMessageQueue.push_back(std::move(message));
    guard.unlock();
    state.cv_sendQueue.notify_one();
}

std::unique_ptr<BaseMessage> receive_message(State& state) {
    std::unique_lock<std::mutex> guard(state.mtx_recvQueue);
    state.cv_recvQueue.wait(guard, [&]() { 
        return state.should_quit || !state.recvMessageQueue.empty(); 
    });
    
    if (state.should_quit) return nullptr;

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


void wait_for_instructions_in_loop(State& state) {
    while (!state.should_quit) {
        std::cout << "Waiting for server instructions..." << std::endl;

        // blocking
        auto msg = receive_message(state);
        
        if (msg == nullptr) {
            break;
        }
        
        switch (msg->GetType()) {
            case BaseMessage::PING: {
                auto resp = std::make_unique<PongMessage>();
                std::cout << "Received PING. Sending PONG." << std::endl;
                send_message(state, std::move(resp));
            } break;
            case BaseMessage::TASK: {
                TaskMessage *msg_ptr = dynamic_cast<TaskMessage*>(msg.get());
                Task task = msg_ptr->task;
                std::cout << "Received TASK. Appending to TaskQueue." << std::endl;

                std::unique_lock<std::mutex> guard(state.mtx_taskQueue);
                state.taskQueue.emplace_back(task);
                guard.unlock();
                state.cv_taskQueue.notify_one();
            } break;
            case BaseMessage::TASK_NOTIFY: {
                TaskNotifyMessage *msg_ptr = dynamic_cast<TaskNotifyMessage*>(msg.get());
                std::cout << "Received NOTIFY. Sending Reply" << std::endl;

                task_id_t task_id = msg_ptr->task_id;
                TaskStatus ts = msg_ptr->task_status;
                
                switch (msg_ptr->task_status) {
                    case TaskStatus::TS_QUESTION: {
                        auto tn_msg = std::make_unique<TaskNotifyMessage>();
                        tn_msg->task_id = state.current_task_id;
                        if (state.current_task_id == TASK_ID_NONE) {
                            tn_msg->task_status = TaskStatus::TS_NONE;
                        } 
                        else {
                            tn_msg->task_status = TaskStatus::TS_RUNNING;
                        }
                        send_message(state, std::move(tn_msg));
                    } break;
                    case TaskStatus::TS_CANCELED: {
                        assert(0 && "<Panic>");
                    } break;
                    default: {
                        assert(0 && "Invalid TaskStatus: <Panic>");
                    }
                }
            } break;
            default: {
                assert(0 && "Not Implemented");
            }
        }
    }
    std::cout << "[WFI]: Waiting for instruction loop broken. Returning." << std::endl;
    terminate_program(state);
}