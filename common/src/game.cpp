#include <string>
#include <filesystem>
#include <vector>
#include <iostream>
#include <fstream>

#include "json.hpp"
#include "game.h"

using json = nlohmann::json;

void to_json(json &j, const Agent& a) {
    j = json{ 
        { "id", a.id, }, 
        { "name" , a.name }, 
        { "filename" , a.filename },  
        { "hash" , a.hash } 
    };
}

void from_json(const json &j, Agent & a) {
    j.at("id").get_to(a.id);
    j.at("name").get_to(a.name);
    j.at("filename").get_to(a.filename);
    j.at("hash").get_to(a.hash);
}

void to_json(json &j, const Game& g) {
    j = json {
        { "id", g.id, }, 
        { "name" , g.name }, 
        { "filename" , g.filename },  
        { "hash" , g.hash },
        { "agents", g.agents }
    };
}

void from_json(const json& j, Game &g) {
    j.at("id").get_to(g.id);
    j.at("name").get_to(g.name);
    j.at("filename").get_to(g.filename);
    j.at("hash").get_to(g.hash);
    j.at("agents").get_to(g.agents);
}