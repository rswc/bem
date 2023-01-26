#include "config.h"
#include "json.hpp"

void to_json(json& j, const CoordinatorConfig& c) {
    j = json { 
        { "port", c.port },  
        { "protocol_version", c.protocol_version },
        { "gamelist", c.gamelist }
    };
}

void from_json(const json& j, CoordinatorConfig& c) {
    j.at("port").get_to(c.port);
    j.at("protocol_version").get_to(c.protocol_version);
    j.at("gamelist").get_to(c.gamelist);
}

bool load_config_from_file(CoordinatorConfig& config, const std::string& configpath = DEFAULT_CONFIG_FILENAME) {
    if (!std::filesystem::exists(configpath)) {
        std::cerr << "[!] Cannot load coordinator config: path <" << configpath << "> does not exist." << std::endl;
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