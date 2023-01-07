#pragma once

#include <string>
#include <vector>

#include "game.h"

struct CoordinatorConfig {
    int port;
    std::string games_dir; 
    std::vector<Game> games;
};

void to_json(json& j, const CoordinatorConfig& c);
void from_json(const json& j, CoordinatorConfig& c);
bool load_config_from_file(const std::string& configpath, CoordinatorConfig& config);