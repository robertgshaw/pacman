#include <iostream>
#include <thread>
#include "nlohmann/json.hpp"

#include "client_helpers.hh"
#include "controller.hh"
#include "utilities.hh"

using json = nlohmann::json;

int main(int argc , char *argv[]) {

	// init and connect to socket
	int sfd = init_socket(); 
    if (sfd == -1) {
		std::cerr << "Shutting down." << std::endl;
		return 1;
	}

	std::cout << "Welcome to pacman." << std::endl << std::endl << "Connecting to server...";

	// get the board from the game	
	char server_msg[BUFSIZ];
	if (recv(sfd, server_msg, BUFSIZ, 0) < 0) {
		std::cerr << " Shutting down..." << std::endl;
		close(sfd);
		return 1;
	}
	
	// TODO: error check the server message  
	std::cout << "Done." << std::endl << "Initalizing game ...";
	json board_json = json::parse(parse_message(server_msg, server_board_format, body_format, body_keyword_len));

	// Initialize the MVC framework
	Controller controller_(board_json);

	// Spin up thread to handle changelog messages from server, controller handles events
	std::thread changelog_t(handle_changelog, sfd, &controller_);

	// Use current thread to handle user commands + send to server
	handle_user(sfd, &controller_);
	
	changelog_t.join();
	close(sfd);
	
	return 0;
}