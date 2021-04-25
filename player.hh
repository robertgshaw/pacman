#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <stdio.h>
#include <unistd.h>
#include "helpers.hh"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

// Player class implements the underlying API

class Player {
    public:
        Player(int client_fd, int ind);
        void handle_connection();

        // API calls
        bool send_board(json board_json);
        bool send_update(json update_json);

    private:
        int cfd;        // socket of connection
        int id;         // player id "index of connection" 

        bool write_to_socket(std::string msg);      // wrapper around C syscalls 
};

#endif