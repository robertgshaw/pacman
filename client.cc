#include <stdio.h>		// printf
#include <string.h>		// strlen
#include <sys/socket.h> // socket
#include <arpa/inet.h>	// inet_addr
#include <unistd.h>
#include <iostream>
#include "helpers.hh"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

int main(int argc , char *argv[])
{
	int port = 6169;
    int sfd;
	struct sockaddr_in server;
	char message[BUFSIZ] , server_reply[BUFSIZ];
	
	// create socket
	sfd = socket(AF_INET , SOCK_STREAM , 0);
	if (sfd == -1) {
		printf("Could not create socket");
	}
	
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	// connect to remote server
	if (connect(sfd, (struct sockaddr*) &server, sizeof(server)) < 0) {
		perror("connect failed. Error");
		return 1;
	}
		
	// send messages to the server
	json j;
	while(1) {
		// get command from the commandline
		printf("Enter message : ");
		scanf("%s" , message);
		j["move"] = message;
		std::string msg_to_send = format_msg(j);
		std::cout << msg_to_send << std::endl;

		// send command to the server
		if (send(sfd, msg_to_send.c_str(), msg_to_send.size(), 0) < 0)	{
			puts("Send failed");
			return 1;
		}
		
		// // Receive a reply from the server
		// if (recv(sfd, server_reply, BUFSIZ, 0) < 0) {
		// 	puts("recv failed");
		// 	break;
		// }
		
		// puts("Server reply :");
		// puts(server_reply);
	}
	
	close(sfd);
	return 0;
}