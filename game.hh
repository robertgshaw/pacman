#ifndef GAME_H
#define GAME_H

#include <vector>
#include <string>
#include <cassert>
#include "board.hh"
#include "player.hh"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class Game {
    public:
        Game(int w);
        void create_player(int cfd);
        Player* get_player(int i);

        json get_board_json();

    private:
        Board board_;                    // board holding the shared state
        std::vector<Player> players_;    // vector of the player sockets currently connected
        int n_players;                  // count of the number of players
};


#endif