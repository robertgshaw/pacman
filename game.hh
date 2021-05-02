#ifndef GAME_H
#define GAME_H

#include <cassert>
#include <string>
#include <tuple>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include "nlohmann/json.hpp"

#include "changelog.hh"
#include "event.hh"
#include "board.hh"

// Server Kernel - holds the data structures and implements synchronization
//
//      Data Structures(private):
//          board_: holds the actual graph represening the gameboard
//          changelog_: hold a queue of the events occuring in the game
//
//      Synchronization Objects (private):
//          b_mutex: mutex used to lock the board
//          cl_mutex: mutex used to lock the changelog_
//          c_cv: condition variable used to notify threads once queue is non-empty
//
//      Data Structure Manipulation (private): update board state / changelog
//          create_player: adds new player to the board --- creates event
//          move_player: moves the player in the board --- create event
//          get_next_event: blocks until new event in the CL queue
//

class Game {
    public:
        Game(int w);
        
        // wrappers around the board API, implementing synchronization
        std::tuple<int, nlohmann::json> create_player(int cfd);
        void move_player(int player_id, int dir);
        
        // wrapper around changelog API, implementing synchronization - gets json of next event
        nlohmann::json get_next_event(int p_id);
        
        // outputting / displaying state
        void print_board(); 
        nlohmann::json get_board_json();

    private:
        int n_players;                  // count of the number of players
        
        // synchronization objects
        std::mutex b_mutex;             // mutex used for the board_
        std::mutex cl_mutex;            // mutex used for the changelog_
        std::condition_variable cl_cv;  // cv used for threads listening to the changelog

        // data structures
        Board board_;                   // board holding the shared state
        Changelog changelog_;           // changelog holding the events of the game
};


#endif