#include "protocol.h"
#include "state.h"

#include "messages.h"
#include "graceful.h"

void send_message(std::unique_ptr<BaseMessage> message) {
    std::unique_lock<std::mutex> guard(getGlobalState().mtx_sendQueue);
    getGlobalState().sendMessageQueue.push_back(std::move(message));
    guard.unlock();
    getGlobalState().cv_sendQueue.notify_one();
}

std::unique_ptr<BaseMessage> receive_message() {
    std::unique_lock<std::mutex> guard(getGlobalState().mtx_recvQueue);
    getGlobalState().cv_recvQueue.wait(guard, [&]() { 
        return getGlobalState().should_quit || !getGlobalState().recvMessageQueue.empty(); 
    });
    
    if (getGlobalState().should_quit) return nullptr;

    auto msg = std::move(getGlobalState().recvMessageQueue.front());
    getGlobalState().recvMessageQueue.pop_front();
    return msg;
}

bool register_to_coordinator() {
    std::cout << "Sending HELLO message to coordinator..." << std::endl;
    getGlobalState().mtx_config.lock();
    auto hello_msg = std::make_unique<HelloMessage>();
    hello_msg->init(getGlobalState().config.protocol_version, HelloMessage::HelloFlag::NONE, getGlobalState().config.gamelist);
    send_message(std::move(hello_msg));
    getGlobalState().mtx_config.unlock();

    bool sync_complete = false;
    while (!sync_complete) {
        std::cout << "Waiting for HELLO message from the coordinator..." << std::endl;

        auto msg = receive_message();
        
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


void wait_for_instructions_in_loop() {

    int sillyCounter = 4;

    while (!getGlobalState().should_quit) {
        std::cout << "Waiting for server instructions..." << std::endl;

        // blocking
        auto msg = receive_message();
        
        if (msg == nullptr) {
            break;
        }
        
        switch (msg->GetType()) {
            case BaseMessage::TASK: {
                TaskMessage *msg_ptr = dynamic_cast<TaskMessage*>(msg.get());
                Task task = msg_ptr->task;
                std::cout << "Received TASK. Appending to TaskQueue." << std::endl;

                std::unique_lock<std::mutex> guard(getGlobalState().mtx_taskQueue);
                getGlobalState().taskQueue.emplace_back(task);
                guard.unlock();
                getGlobalState().cv_taskQueue.notify_one();
            } break;
            case BaseMessage::TASK_NOTIFY: {
                TaskNotifyMessage *msg_ptr = dynamic_cast<TaskNotifyMessage*>(msg.get());
                std::cout << "Received NOTIFY. Sending Reply" << std::endl;

                task_id_t task_id = msg_ptr->task_id;
                TaskStatus ts = msg_ptr->task_status;
                
                switch (msg_ptr->task_status) {
                    case TaskStatus::TS_QUESTION: {
                        auto tn_msg = std::make_unique<TaskNotifyMessage>();
                        tn_msg->task_id = getGlobalState().current_task_id;
                        
                        if (getGlobalState().current_task_id == TASK_ID_NONE) {
                            tn_msg->task_status = TaskStatus::TS_NONE;
                        } 
                        else {
                            tn_msg->task_status = TaskStatus::TS_RUNNING;
                        }
                        
                        if (--sillyCounter > 0) // DEBUG: simulate connection break
                            send_message(std::move(tn_msg));
                    } break;
                    case TaskStatus::TS_CANCELLED: {
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
    terminate_program();
}