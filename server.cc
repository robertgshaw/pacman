#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "helpers.hh"
#include "game.hh"
#include "nlohmann/json.hpp"

#define PORT 6169
#define BOARD_SIZE 3
#define MAX_CONNECTIONS 5

using json = nlohmann::json;

void handle_connection(Player* p, int cfd, Game* g) {
    
    char buf[BUFSIZ];
    int read_size;

	// Receive a message from client
	while ((read_size = recv(cfd, buf, BUFSIZ, 0)) > 0) {
        if (!p->send_board(g->get_board_json())) {
            
            puts("Could not send board to client\n");
            fflush(stdout);

            close(cfd);
            
            return;
        }
	}
	
    // checking why 
	if (read_size == 0) {
		puts("Player disconnected\n");
		fflush(stdout);

	} else if (read_size == -1) {
		perror("Recv failed");
	}

    close(cfd);
    return;
}

int main(int argc, char** argv) {

    // create socket, bind, and listen 
    int sfd = init_socket(PORT, MAX_CONNECTIONS);
    if (sfd == -1) {
        printf("\n\n Could not init socket ... shutting down \n\n");
        return 1;
    } else {
        printf("\n\nListening on port %d ... \n\n", PORT);
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
    game_.create_player(cfd);

    handle_connection(game_.get_player(0), cfd, &game_);

    close(sfd);

	return 0;
}