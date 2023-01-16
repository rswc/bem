#include "config.h"
#include "json.hpp"


void to_json(json& j, const NodeConfig& c) {
    j = json { 
        { "host", c.host }, 
        { "port", c.port }, 
        { "protocol_version", c.protocol_version },
        { "games_dir", c.games_dir }, 
        { "gamelist", c.gamelist }
    };
}

void from_json(const json& j, NodeConfig& c) {
    j.at("host").get_to(c.host);
    j.at("port").get_to(c.port);
    j.at("protocol_version").get_to(c.protocol_version);
    j.at("games_dir").get_to(c.games_dir);
    j.at("gamelist").get_to(c.gamelist);
}

bool load_node_config_from_file(NodeConfig& config, const std::string& configpath) {
    if (!std::filesystem::exists(configpath)) {
        std::cout << "[!] Cannot load node config: path <" << configpath << "> does not exist." << std::endl;
        return false;
    }

    std::ifstream ifs(configpath);
    config = json::parse(ifs);
    return true;
}