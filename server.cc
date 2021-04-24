#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "board.hh"

#define PORT 6169

// main
//      accepts connections on the sockets

int main(int argc, char** argv) {

    int sfd, cfd, c, read_size;
    struct sockaddr_in server, client;
    char client_message[2000];
    
    // create socket
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1) {
        printf("Could not create socket\n");
        return 1;
    } 

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // bind socket
    if (bind(sfd, (struct sockaddr*) &server, sizeof(server)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(sfd, 3) < 0) {
        perror("listen");
        return 1;
    }

    printf("Listening on port %d ... \n", PORT);

    Board board = Board(5);
    board.print_board(1);

    c = sizeof(struct sockaddr_in);
    cfd = accept(sfd, (struct sockaddr*) &client, (socklen_t*)&c);
    if (cfd < 0) {
        perror("accept");
        return 1;
    }
    
	//Receive a message from client
	while ((read_size = recv(cfd, client_message, 2000, 0)) > 0) {
		//Send the message back to client
		write(cfd, client_message, strlen(client_message));
	}
	
	if (read_size == 0) {
		puts("Client disconnected");
		fflush(stdout);
	} else if (read_size == -1) {
		perror("recv failed");
	}
	
    close(cfd);
    close(sfd);

	return 0;
}