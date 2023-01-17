#pragma once

#include <string>
#include <vector>

#include "gamelist.h"

#define DEFAULT_CONFIG_FILENAME "node.json"
struct NodeConfig {
    std::string host;
    uint16_t port;
    uint8_t protocol_version;
    std::string games_dir; 
    GameList gamelist;
};

void to_json(json& j, const NodeConfig& c);
void from_json(const json& j, NodeConfig& c);
bool load_node_config_from_file(NodeConfig& config, const std::string& configpath);