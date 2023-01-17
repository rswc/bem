#include "maintenance.h"
#include "taskNotifyMessage.h"

void doMaintenance(State &state) {

    while (!state.shouldQuit)
    {
        state.mtx_nodes.lock();
        
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
                }

            } else if (node->awaiting_response() && node->time_from_request() > 5. && !(node->flags & NodeFlag::CONN_BROKEN)) {
                
                // if still awaiting response from previous ping,
                // consider the connection (temporarily?) broken

                node->flags |= NodeFlag::CONN_BROKEN;
                // TODO: Handle disconnect, reassign this node's tasks

                std::cout << "[MNCE] Connection with Node #" << nid << " broken \n";

            }

        }

        state.mtx_nodes.unlock();

        // TODO: read from config?
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

}
