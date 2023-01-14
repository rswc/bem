#pragma once

#include <string>
#include <vector>

#include "gamelist.h"

struct NodeConfig {
    std::string host;
    short port;
    uint8_t protocol_version;
    std::string games_dir; 
    GameList gamelist;
};

void to_json(json& j, const NodeConfig& c);
void from_json(const json& j, NodeConfig& c);
bool load_config_from_file(const std::string& configpath, NodeConfig& config);