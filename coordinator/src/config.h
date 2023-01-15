#pragma once

#include <string>
#include <vector>

#include "gamelist.h"

struct CoordinatorConfig {
    uint16_t port;
    uint8_t protocol_version;
    std::string games_dir; 
    GameList gamelist;
};

extern const std::string DEFAULT_CONFIG_FILENAME; 

void to_json(json& j, const CoordinatorConfig& c);
void from_json(const json& j, CoordinatorConfig& c);
bool load_config_from_file(CoordinatorConfig& config, const std::string& configpath);
