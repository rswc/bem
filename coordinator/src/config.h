#pragma once

#include <string>
#include <vector>

#include "gamelist.h"

#define DEFAULT_CONFIG_FILENAME "coordinator.json" 

struct CoordinatorConfig {
    uint16_t port;
    uint64_t node_broken_seconds;
    uint8_t protocol_version;
    GameList gamelist;
};

void to_json(json& j, const CoordinatorConfig& c);
void from_json(const json& j, CoordinatorConfig& c);
bool load_config_from_file(CoordinatorConfig& config, const std::string& configpath);
