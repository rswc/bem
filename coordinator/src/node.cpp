#include "node.h"

#include <unistd.h>
#include <error.h>
#include <string>

const node_id_t NODE_ID_NONE = 0u;
const node_id_t NODE_ID_FIRST = 1u;

void Node::AssignTask(std::shared_ptr<Task> task)
{
    tasks.push_back(task);
    // message node
}

void Node::Send(std::unique_ptr<BaseMessage> message)
{
    std::unique_lock<std::mutex> guard(mtx_msgQueue);
    messageQueue.push_back(std::move(message));
    guard.unlock();
    cv_msgQueue.notify_one();
}
