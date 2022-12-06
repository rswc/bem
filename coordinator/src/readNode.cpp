#include "readNode.h"

#include <unistd.h>
#include <error.h>

void readNode(int sock, std::shared_ptr<Node> node, State& state)
{
    char buf;
    while (!state.shouldQuit)
    {
        auto len = read(sock, &buf, 1);
	    if (len == -1) 
        {
            error(1, errno, "read failed on node %d", node->id);
        }

        if (buf != '\n')
        {
            node->lastCh = buf;
        }
    }
}
