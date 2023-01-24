#include "maintenance.h"
#include "taskNotifyMessage.h"
#include "node.h"

void doMaintenance(State &state) {

    bool tasksNeedBalancing;

    while (!state.shouldQuit)
    {
        tasksNeedBalancing = false;

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
                }

            } else if (node->time_from_request() > 5. && !(node->flags & NodeFlag::CONN_BROKEN)) {
                
                // if still awaiting response from previous ping,
                // consider the connection (temporarily?) broken

                node->flags |= NodeFlag::CONN_BROKEN;

                // The node's tasks are unassigned, so if connection is restored,
                // it should receive the instruction to cancel the task
                for (auto& task : node->tasks) {
                    if (task->status == TS_RUNNING) {
                        task->status = TS_NONE;

                        node->UnassignTask(task);

                        tasksNeedBalancing = true;
                    }
                }

                std::cout << "[MNCE] Connection with Node #" << nid << " broken \n";

            }
            // TODO: for example:
            // else if (node->time_from_request() > 3600. && (node->flags & NodeFlag::CONN_BROKEN)) {
            //     // shutdown, remove node from state
            // }

        }

        if (tasksNeedBalancing) {
            balanceTasks(state);
        }

        state.mtx_nodes.unlock();
        state.mtx_tasks.unlock();

        // TODO: read from config?
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

}
