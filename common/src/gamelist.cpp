#include <string>
#include <filesystem>
#include <vector>
#include <iostream>
#include <fstream>

#include "json.hpp"
#include "gamelist.h"

using json = nlohmann::json;

void to_json(json &j, const Agent& a) {
    j = json{ 
        { "id", a.agent_id, }, 
        { "name" , a.name }, 
        { "filename" , a.filename },  
        { "hash" , a.hash } 
    };
}

void from_json(const json &j, Agent & a) {
    j.at("id").get_to(a.agent_id);
    j.at("name").get_to(a.name);
    j.at("filename").get_to(a.filename);
    j.at("hash").get_to(a.hash);
}

void to_json(json &j, const Game& g) {
    
    std::vector<Agent> agents;
    for (const auto &p : g.agents) {
        agents.push_back(p.second);
    }

    j = json {
        { "id", g.game_id, }, 
        { "name" , g.name }, 
        { "filename" , g.filename },  
        { "hash" , g.hash },
        { "agents", agents }
    };
}

void from_json(const json& j, Game &g) {
    j.at("id").get_to(g.game_id);
    j.at("name").get_to(g.name);
    j.at("filename").get_to(g.filename);
    j.at("hash").get_to(g.hash);
    
    std::vector<Agent> agents;
    j.at("agents").get_to(agents);
    
    for (const Agent& a : agents) {
        g.agents.insert({a.agent_id, a});
    }
}

void to_json(json &j, const GameList& gl) {
    std::vector<Game> games;

    for (const auto &p : gl.games) {
        games.push_back(p.second);
    }

    j = json { { "games", games } };
}

void from_json(const json& j, GameList &gl) {
    std::vector<Game> games;
    j.at("games").get_to(games);

    for (const Game& g : games) {
        gl.games.insert({g.game_id, g});
    }
}

void GameList::print() const {
    std::cout << "---- GameList ----\n";
    for (const auto &gp : games) {
        std::cout << "-- [" << gp.first << "]: " << gp.second.name << std::endl; 
        
        for (const auto &ap : gp.second.agents) {
            std::cout << "[" << ap.first << "]: " << ap.second.name << std::endl; 
        }
    }
    std::cout << "--------\n";
}