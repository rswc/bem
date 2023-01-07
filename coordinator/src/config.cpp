#include "config.h"
#include "json.hpp"

void to_json(json& j, const CoordinatorConfig& c) {
    j = json { { "port", c.port },  {"games_dir", c.games_dir}, { "games", c.games }};
}

void from_json(const json& j, CoordinatorConfig& c) {
    j.at("port").get_to(c.port);
    j.at("games_dir").get_to(c.games_dir);
    j.at("games").get_to(c.games);
}

bool load_config_from_file(const std::string& configpath, CoordinatorConfig& config) {
    if (!std::filesystem::exists(configpath)) {
        std::cout << "[!] Cannot load coordinator config: path <" << configpath << "> does not exist." << std::endl;
        return false;
    }

    std::ifstream ifs(configpath);
    config = json::parse(ifs);
    return true;
}