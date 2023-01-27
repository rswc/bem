#include "maintenance.h"
#include "taskNotifyMessage.h"
#include "node.h"

void doMaintenance(State &state) {
    

    state.mtx_config.lock();
    double threshold_s = (double) state.config.node_broken_seconds;
    state.mtx_config.unlock();

    // std::cout << "[MT]: Starting maintenance loop with " << threshold_s << " [s] threshold for broken connections." << std::endl;

    while (!state.shouldQuit)
    {
        state.mtx_nodes.lock();
        state.mtx_tasks.lock();
        
        for (auto& [nid, node] : state.nodes) {

            if (!node->awaiting_response()) {

                // ping node
                auto msg = std::make_unique<TaskNotifyMessage>();
                msg->task_id = TASK_ID_NONE;
                msg->task_status = TS_QUESTION;
            
                node->mark_request();
                node->Send(std::move(msg));

                // Connection restored
                if (node->flags & NodeFlag::CONN_BROKEN) {
                    node->flags &= ~NodeFlag::CONN_BROKEN;

                    std::cout << "[MNCE] Connection with Node #" << nid << " restored \n";
                    // Probably node can take work of others?
                    state.suspectedBalancing = true;
                }

            } else if (node->time_from_request() > threshold_s && !(node->flags & NodeFlag::CONN_BROKEN)) {
                
                // if still awaiting response from previous ping,
                // consider the connection (temporarily?) broken

                node->flags |= NodeFlag::CONN_BROKEN;

                // The node's tasks are unassigned, so if connection is restored,
                // it should receive the instruction to cancel the task
                
                std::vector<task_id_t> mark_for_delete;

                for (auto& task : node->tasks) {
                    if (task->status == TS_RUNNING) {
                        task->status = TS_NONE;
                        mark_for_delete.push_back(task->id);
                        // does not require a mutex
                    }
                }
                
                for (task_id_t task_id : mark_for_delete) {
                    node->UnassignTask(task_id);
                }

                if (!mark_for_delete.empty()) {
                    state.suspectedBalancing = true;
                }

                std::cout << "[MNCE] Connection with Node #" << nid << " broken \n";

            }
            // TODO: for example:
            // else if (node->time_from_request() > 3600. && (node->flags & NodeFlag::CONN_BROKEN)) {
            //     // shutdown, remove node from state
            // }
        }

        state.mtx_nodes.unlock();
        state.mtx_tasks.unlock();

        if (state.suspectedBalancing) {
            balanceTasks(state);
        }


        // TODO: read from config?
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

}
