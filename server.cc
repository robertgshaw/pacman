#include <stdio.h>
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "helpers.hh"
#include "game.hh"
#include "nlohmann/json.hpp"

#define PORT 6169
#define BOARD_SIZE 4
#define MAX_CONNECTIONS 5

using json = nlohmann::json;

int main(int argc, char** argv) {

    // create socket, bind, and listen 
    int sfd = init_socket(PORT, MAX_CONNECTIONS);
    if (sfd == -1) {
        printf("Could not init socket...shutting down\n");
        return 1;
    } else {
        printf("Listening on port %d...\n", PORT);
    }

    // create game obj, which will hold the shared state
    Game game_ = Game(BOARD_SIZE);

    // accept client connection, blocking while waiting for connection
    socklen_t c = sizeof(struct sockaddr);
    struct sockaddr client;
    int cfd = accept(sfd, &client, &c);
    if (cfd < 0) {
        perror("accept");
        return 1;
    }
    
    // add new_player to the game
    int ind = 0; // ctr, to grow as we add more connections
    Player* p_ptr = game_.create_player(cfd);
    p_ptr->handle_connection();

    close(sfd);

	return 0;
}