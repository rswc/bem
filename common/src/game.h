#pragma once

#include <string>
#include <filesystem>
#include <vector>
#include <iostream>
#include <fstream>

#include "json.hpp"

using json = nlohmann::json;

struct Agent {
    std::uint64_t id;
    std::string name;
    std::string filename;
    std::string hash;
};

struct Game {
    std::uint64_t id;
    std::string name; 
    std::string filename;
    std::string hash;
    std::vector<Agent> agents;
};

void to_json(json &j, const Agent& a);
void from_json(const json &j, Agent & a);
void to_json(json &j, const Game& g);
void from_json(const json& j, Game &g);
