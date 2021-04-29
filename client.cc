// #include <stdio.h>		// printf
#include <sys/socket.h> // socket
#include <arpa/inet.h>	// inet_addr
#include <unistd.h>
#include <iostream>
#include "client_helpers.hh"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

static const int port = 6169;
static const char* path = "127.0.0.1";

int main(int argc , char *argv[]) {

    int sfd;
	struct sockaddr_in server;
	std::string message;
	char server_reply[BUFSIZ];
	
	// create socket
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1) {
		std::cout << "Could not create socket";
		return 1;
	}
	
	server.sin_addr.s_addr = inet_addr(path);
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	// connect to remote server
	if (connect(sfd, (struct sockaddr*) &server, sizeof(server)) < 0) {
		std::cerr << "Connect failed. Error";
		return 1;
	}
		
	// send messages to the server
	while (true) {

		// get command from the commandline
		std::cout << "Enter move: ";
		std::cin >> message;

		json j;
		j["move"] = message;
		std::string msg_to_send = format_client_request(j);

		// send command to the server
		if (send(sfd, msg_to_send.c_str(), msg_to_send.size(), 0) < 0)	{
			std::cerr << "Send failed";
			close(sfd);
			return 1;
		}
		
		// receive a reply from the server
		if (recv(sfd, server_reply, BUFSIZ, 0) < 0) {
			std::cerr << "Recv failed";
			close(sfd);
			return 1;
		}

		std::cout << "Server reply: " << server_reply << std::endl;
	}
	
	close(sfd);
	return 0;
}