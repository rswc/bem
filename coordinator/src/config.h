#pragma once

#include <string>
#include <vector>

#include "gamelist.h"

struct CoordinatorConfig {
    int port;
    uint8_t protocol_version;
    std::string games_dir; 
    GameList gamelist;
};

void to_json(json& j, const CoordinatorConfig& c);
void from_json(const json& j, CoordinatorConfig& c);
bool load_config_from_file(const std::string& configpath, CoordinatorConfig& config);