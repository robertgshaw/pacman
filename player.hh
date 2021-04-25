#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <stdio.h>
#include <unistd.h>

// Player class implements the underlying API

class Player {
    public:
        Player(int client_fd, int ind);

        // API calls
        bool send_board(std::string board_json);

    private:
        int cfd;        // socket of connection
        int id;         // player id "index of connection" 

        // helper function for underlying sys calls
        bool write_to_socket(std::string msg); 
};

#endif