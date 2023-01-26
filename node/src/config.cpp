#include "config.h"
#include "json.hpp"


void to_json(json& j, const NodeConfig& c) {
    j = json { 
        { "host", c.host }, 
        { "port", c.port }, 
        { "protocol_version", c.protocol_version },
        { "games_dir", c.games_dir }, 
        { "game_executable", c.game_launcher }, 
        { "gamelist", c.gamelist }
    };
}

void from_json(const json& j, NodeConfig& c) {
    j.at("host").get_to(c.host);
    j.at("port").get_to(c.port);
    j.at("protocol_version").get_to(c.protocol_version);
    j.at("games_dir").get_to(c.games_dir);
    j.at("game_launcher").get_to(c.game_launcher);
    j.at("gamelist").get_to(c.gamelist);
}

bool load_node_config_from_file(NodeConfig& config, const std::string& configpath) {
    if (!std::filesystem::exists(configpath)) {
        std::cerr << "[!] Cannot load node config: path <" << configpath << "> does not exist." << std::endl;
        return false;
    }

    try {
        std::ifstream ifs(configpath);
        config = json::parse(ifs);
    } catch(json::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }

    return true;
}