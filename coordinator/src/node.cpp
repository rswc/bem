#include "node.h"

#include <unistd.h>
#include <error.h>
#include <string>

#include "state.h"
#include "taskMessage.h"
#include "taskNotifyMessage.h"


const node_id_t NODE_ID_NONE = 0u;
const node_id_t NODE_ID_FIRST = 1u;

void Node::AssignTask(std::shared_ptr<Task> task)
{
    tasks.push_back(task);
    
    auto msg = std::make_unique<TaskMessage>();
    msg->task = *task;

    Send(std::move(msg));
}

void Node::UnassignTask(std::shared_ptr<Task> task)
{
    auto it_task = std::find(tasks.begin(), tasks.end(), task);

    if (it_task == tasks.end())
        return;
    
    tasks.erase(it_task);

    auto msg = std::make_unique<TaskNotifyMessage>();
    msg->task_id = task->id;
    msg->task_status = TS_CANCELLED;

    Send(std::move(msg));
}

void Node::Send(std::unique_ptr<BaseMessage> message)
{
    std::unique_lock<std::mutex> guard(mtx_msgQueue);
    messageQueue.push_back(std::move(message));
    guard.unlock();
    cv_msgQueue.notify_one();
}

std::vector<node_id_t> get_eligible_nodes_for_task(State& state, const Task& task) {
    std::vector<node_id_t> eligible_ids;
    
    std::cout << "Given task: " << task.game_id << ", " << task.agent1 << ", " << task.agent2 << std::endl;
    state.mtx_nodes.lock();
    for (const auto&[node_id, node] : state.nodes) {
        
        std::cout << "Hello: " << node_id << ", " << node->is_registered() << ", " << node->gamelist.contains_game(task.game_id) << std::endl;
        
        if (!node->is_registered()) continue;
        if (node->flags & NodeFlag::CONN_BROKEN) continue;
        if (!node->gamelist.contains_game(task.game_id)) continue;
        
        
        if (node->gamelist.contains_agent(task.game_id, task.agent1) 
            && node->gamelist.contains_agent(task.game_id, task.agent2)) {
        
            eligible_ids.push_back(node_id);
        }
        
    }
    state.mtx_nodes.unlock();
    return eligible_ids;
}

void balanceTasks(State& state) {

    // This function should be called whenever we suspect
    // the distribution of tasks among nodes to have changed
    // -- when connection to node is broken or restored, 
    // when a node finishes some tasks, a new node registers, etc.

    state.mtx_nodes.lock();
    state.mtx_tasks.lock();

    // TODO: split & balance unassigned tasks between nodes (somehow)
    
    for (auto& [tid, task] : state.tasks) {
        
        if (task->status == TS_NONE) {

            // get eligible nodes zajmuje mutex
            state.mtx_nodes.unlock();
            auto node_ids = get_eligible_nodes_for_task(state, *task);
            state.mtx_nodes.lock();

            if (node_ids.empty()) continue;

            // TODO: better algorithm for finding most suitable node
            state.nodes[node_ids[0]]->AssignTask(task);

        }

    }

    state.mtx_nodes.unlock();
    state.mtx_tasks.unlock();
}
