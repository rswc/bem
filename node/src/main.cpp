#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <error.h>
#include <netdb.h>
#include <thread>
#include <iostream>
#include <chrono>

#include "messages.h"

#include "state.h"
#include "config.h"
#include "protocol.h"
#include "communication.h"


void doTasks(State &state) {
    while (true) {
        std::unique_lock<std::mutex> guard(state.mtx_taskQueue);
        state.cv_taskQueue.wait(guard, [&]{
            return !state.taskQueue.empty();
        });
        Task task = state.taskQueue.front();
        state.taskQueue.pop_front();
        guard.unlock();
        
        state.current_task_id = task.id;
        std::cout << "[DT]: Task with id " << task.id << ", received. Sleeping for 10 seconds" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::cout << "[DT]: Task with id " << task.id << ", done. Sending result" << std::endl;
        
        auto result_msg = std::make_unique<ResultMessage>();
        result_msg->init(task.id);
        send_message(state, std::move(result_msg));
        state.current_task_id = TASK_ID_NONE;
    }
}

int main(int argc, char const *argv[]) {
    std::string configpath = std::string(DEFAULT_CONFIG_FILENAME);
    if (argc == 2) {
        configpath = std::string(argv[1]);
    } 

    State state;

    // no mutex needed, we dont assume multihtreading here yet
    bool setup_ok = load_node_config_from_file(state.config, configpath);
    if (!setup_ok) {
        std::cerr << "[!] Reading configuration file resulted in failure. Make sure JSON config is valid." << std::endl;
        return 1;
    }

    // set socket as state variable
    bool connection_established = connect_to_server(state);
    if (!connection_established) {
        std::cerr << "[!] Connection to coordinator could not be establised. Make sure server is up and running." << std::endl;
        return 1;
    }

    // graceful exit?
    int sock = state.socket;
    std::thread t_write(write_to_server_in_loop, std::ref(state), sock);
    std::thread t_read(read_from_server_in_loop, std::ref(state), sock, 32);
    std::thread t_task(doTasks, std::ref(state));

    bool register_ok = register_to_coordinator(state);
    if (!register_ok) {
        // TODO: graceful exit?
        std::cerr << "[!] Hello-Handshake with coordinator was not completed." << std::endl;
        return 1;
    }

    while (true) {
        std::cout << "Waiting for server instructions..." << std::endl;
        auto msg = receive_message(state);
        
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
        if (msg->GetType() == BaseMessage::PING) {
            // TODO: check for contents? 
            
            auto resp = std::make_unique<PongMessage>();
            std::cout << "Received PING. Sending PONG." << std::endl;
            send_message(state, std::move(resp));
        }
        else if (msg->GetType() == BaseMessage::TASK) {
            TaskMessage *msg_ptr = dynamic_cast<TaskMessage*>(msg.get());
            Task task = msg_ptr->task;
            std::cout << "Received TASK. Appending to TaskQueue." << std::endl;

            std::unique_lock<std::mutex> guard(state.mtx_taskQueue);
            state.taskQueue.emplace_back(task);
            guard.unlock();
            state.cv_taskQueue.notify_one();
        }
    }

    t_write.join();
    t_read.join();
    t_task.join();

    close(state.socket);

    return 0;
}

