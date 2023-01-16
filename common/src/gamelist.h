#pragma once

#include <string>
#include <filesystem>
#include <vector>
#include <iostream>
#include <fstream>

#include "json.hpp"

using json = nlohmann::json;

using games_id_t = uint64_t;
extern const games_id_t GAME_ID_NONE;
extern const games_id_t GAME_ID_FIRST;

struct Agent {
    games_id_t agent_id;
    std::string name;
    std::string filename;
    std::string hash;
};

struct Game {
    games_id_t game_id;
    std::string name; 
    std::string filename;
    std::string hash;
    std::unordered_map<games_id_t, Agent> agents;
};

class GameList {
private:

public:
    void print() const;
    std::unordered_map<games_id_t, Game> games;
    
    bool contains_game(games_id_t game_id) { return games.find(game_id) != games.end(); }
    bool contains_agent(games_id_t game_id, games_id_t agent_id) {
        auto it = games.find(game_id);
        if (it == games.end()) return false;
        else return it->second.agents.find(agent_id) != it->second.agents.end();
    }
};

void to_json(json &j, const Agent& a);
void from_json(const json &j, Agent & a);
void to_json(json &j, const Game& g);
void from_json(const json& j, Game &g);
void to_json(json &j, const GameList& gl);
void from_json(const json& j, GameList &gl);
