#ifndef GAME_H
#define GAME_H

#include <mutex>
#include <vector>
#include <string>
#include <cassert>
#include "nlohmann/json.hpp"

#include "board.hh"
// #include "player.hh"

using json = nlohmann::json;

class Player; // forward declaration

class Game {
    public:
        Game(int w);
        
        int create_player(int cfd);
        // Player* get_player(int i);
        void move_player(int player_id, int dir);
        
        void print_board(); 
        json get_board_json();

    private:

        // std::mutex g_mutex;             // mutex used to lock the player vector
        Board board_;                   // board holding the shared state
        // std::vector<Player> players_;   // vector holding the players
        int n_players;                  // count of the number of players
};


#endif