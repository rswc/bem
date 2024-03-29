#pragma once

#include <netinet/in.h>
#include <arpa/inet.h>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <deque>
#include <chrono>

#include "task.h"
#include "baseMessage.h"
#include "gamelist.h"

struct State;

using node_id_t = uint32_t;
extern const node_id_t NODE_ID_NONE;
extern const node_id_t NODE_ID_FIRST;

using node_flag_t = uint8_t;
enum NodeFlag : node_flag_t {
    FLAGS_CLEAR = 0,
    REGISTERED = 1u << 0,
    CONN_BROKEN = 1u << 1,
    FLAGS_N_BITS = 3,
};

std::vector<node_id_t> get_eligible_nodes_for_task(State& state, const Task& task);

void balanceTasks(State& state);

class Node
{
    friend void writeNode(int sock, std::shared_ptr<Node> node, State& state);
    friend void readNode(int sock, std::shared_ptr<Node> node, State& state);

private:
    std::mutex mtx_msgQueue;
    std::condition_variable cv_msgQueue;
    

public:
    Node() { std::memset(&addr, 0, sizeof(sockaddr_in)); }

    std::vector<std::shared_ptr<Task>> tasks; 
    int socket;
    GameList gamelist;
    node_id_t id = NODE_ID_NONE;
    sockaddr_in addr;
    socklen_t addrSize = sizeof(addr);
    task_id_t activeTaskGroup = TASK_ID_NONE;
    
    std::chrono::time_point<std::chrono::system_clock> response_ts, request_ts; 

    node_flag_t flags = NodeFlag::FLAGS_CLEAR;
    
    std::deque<std::unique_ptr<BaseMessage>> messageQueue;

    void AssignTask(std::shared_ptr<Task> task);
    void UnassignTask(task_id_t id_to_delete);
    void Send(std::unique_ptr<BaseMessage> message);

    bool is_registered() const { return (flags & NodeFlag::REGISTERED); } 
    void mark_registered() { flags |= NodeFlag::REGISTERED; }
    
    void mark_request() { request_ts = std::chrono::system_clock::now(); }  
    void mark_response() { response_ts = std::chrono::system_clock::now(); }
    double reply_time() const { 
        std::chrono::duration<double> elapsed = response_ts - request_ts;
        return elapsed.count();
    }
    double time_from_request() const { 
        std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - request_ts;
        return elapsed.count();
    }
    double time_from_response() const { 
        std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - response_ts;
        return elapsed.count();
    }
    bool awaiting_response() const {
        return response_ts < request_ts;
    }

    bool is_idle() const {
        return activeTaskGroup == TASK_ID_NONE;
    }
};

