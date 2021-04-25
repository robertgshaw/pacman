#include <stdio.h>	//printf
#include <string.h>	//strlen
#include <sys/socket.h>	//socket
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>

int main(int argc , char *argv[])
{
	int port = 6169;
    int sfd;
	struct sockaddr_in server;
	char message[BUFSIZ] , server_reply[BUFSIZ];
	
	//Create socket
	sfd = socket(AF_INET , SOCK_STREAM , 0);
	if (sfd == -1) {
		printf("Could not create socket");
	}
	
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	//Connect to remote server
	if (connect(sfd, (struct sockaddr*) &server, sizeof(server)) < 0) {
		perror("connect failed. Error");
		return 1;
	}
		
	//keep communicating with server
	while(1)
	{
		printf("Enter message : ");
		scanf("%s" , message);
		
		//Send some data
		if (send(sfd, message, strlen(message), 0) < 0)	{
			puts("Send failed");
			return 1;
		}
		
		//Receive a reply from the server
		if (recv(sfd, server_reply, BUFSIZ, 0) < 0) {
			puts("recv failed");
			break;
		}
		
		puts("Server reply :");
		puts(server_reply);
	}
	
	close(sfd);
	return 0;
}