#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include "helpers.hh"
#include "game.hh"
#include "nlohmann/json.hpp"

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

using json = nlohmann::json;

class Game;

class Player {
    public:
        Player(int client_fd, int ind, Game* g);
        void handle_connection();

        // API calls
        bool send_board(json board_json);
        bool send_update(json update_json);

    private:
        Game* g_ptr;    // pointer to the shared game object

        int cfd;        // socket of connection
        int id;         // player id "index of connection" 

        void handle_request(json request);      // handles actual request from client
        bool write_to_socket(std::string msg);  // wrapper around C syscalls 
};

#endif