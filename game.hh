#ifndef GAME_H
#define GAME_H

#include <vector>
#include <string>
#include <cassert>
#include "board.hh"
#include "player.hh"

class Game {
    public:
        Game(int w);
        void create_player(int cfd);
        std::string get_board_json();
        Player* get_player(int i);

    private:
        Board board_;                    // board holding the shared state
        std::vector<Player> players_;    // vector of the player sockets currently connected
        int n_players;                  // count of the number of players
};


#endif