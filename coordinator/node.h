#pragma once

#include <netinet/in.h>
#include <arpa/inet.h>

enum NodeFlags {
    NONE,
    REGISTERED = 1
};

struct Node
{
    int id = -1;
    sockaddr_in addr{0};
    socklen_t addrSize = sizeof(addr);
    int flags = NONE;

    char lastCh = ' ';
};
