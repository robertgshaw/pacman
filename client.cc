#include <iostream>
#include <thread>
#include "nlohmann/json.hpp"

#include "utilities.hh"
#include "board.hh"
#include "client_helpers.hh"

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
	
	// initialize the game
	std::cout << "Done." << std::endl << "Initalizing game ...";
	std::string board_str = parse_message(server_msg, server_board_format, body_format, body_keyword_len);
	Board board_;
	board_.init_graph(json::parse(board_str)); // TODO: add error checking on the json	
	std::cout << "Done. Starting up..." << std::endl << std::endl;
	board_.print();

	// spin off thread to listen to the server
	std::thread t(handle_changelog, sfd, &board_);
	t.detach();

	handle_user(sfd);
	close(sfd);
	
	return 0;
}