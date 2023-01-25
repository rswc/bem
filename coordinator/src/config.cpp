#include "config.h"
#include "json.hpp"

void to_json(json& j, const CoordinatorConfig& c) {
    j = json { 
        { "port", c.port },  
        { "protocol_version", c.protocol_version },
        { "games_dir", c.games_dir }, 
        { "gamelist", c.gamelist }
    };
}

void from_json(const json& j, CoordinatorConfig& c) {
    j.at("port").get_to(c.port);
    j.at("protocol_version").get_to(c.protocol_version);
    j.at("games_dir").get_to(c.games_dir);
    j.at("gamelist").get_to(c.gamelist);
}

bool load_config_from_file(CoordinatorConfig& config, const std::string& configpath = DEFAULT_CONFIG_FILENAME) {
    if (!std::filesystem::exists(configpath)) {
        std::cout << "[!] Cannot load coordinator config: path <" << configpath << "> does not exist." << std::endl;
        return false;
    }

    std::ifstream ifs(configpath);
    config = json::parse(ifs);
    return true;
}