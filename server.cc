#include <stdio.h>
#include <iostream>
#include <thread>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "nlohmann/json.hpp"
#include "helpers.hh"
#include "game.hh"

static const int port = 6169;
static const int board_size = 4;
static const int max_players =  5;

using json = nlohmann::json;

int main(int argc, char** argv) {

    // create socket, bind, and listen 
    int sfd = init_socket(port, max_players);
    if (sfd == -1) {
        std::cout << "Could not init socket...shutting down\n";
        return 1;
    } else {
        std::cout << "Listening on port " << std::to_string(port) << "..." << std::endl;
    }

    // create game obj, which will hold the shared state
    Game game_(board_size);

    // vars used for the socket
    socklen_t c = sizeof(struct sockaddr);
    struct sockaddr_in client;

    // vars used to init a connection
    int player_id;
    json board_json;

    // accept new client connections, blocking while waiting 
    while(true) {
        int cfd = accept(sfd, (sockaddr*) &client, &c);
        if (cfd < 0) {

            // TODO: tell threads that we are dead + that it should shut down
            perror("accept");
            close(cfd);
            close(sfd);
            return 1;
        }

        // create player + launch thread to listen to CL + Player Commands
        std::tie(player_id, board_json) = game_.handle_add_player(cfd);
        std::thread t(handle_connection, cfd, player_id, &game_, board_json);
        t.detach();

    } 

    close(sfd);
	return 0;
}