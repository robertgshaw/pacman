#include <iostream>
#include <cstdlib>
#include <thread>
#include <string>
#include <tuple>
#include <cassert>
#include "../shared/nlohmann/json.hpp"

#include "client_api.hh"
#include "controller.hh"
#include "../shared/utilities.hh"

using json = nlohmann::json;

int main(int argc , char *argv[]) {
	
	// TODO - these need to be passes as arguments to init_socket()
	int port;
	char* path;

	if (argc != 3) {
		std::cout << "usage: ./client [server-ip] [port]" << std::endl;
		return 0;
	} else {
		path = argv[1];
		port = atoi(argv[2]);
	}

	// init and connect to socket
	int sfd = init_socket(port, path); 
    if (sfd == -1) {
		std::cerr << "Error: failed to init socket." << std::endl;
		return 1;
	}

	// get the board from the game	
	char server_msg[BUFSIZ];
	std::string start_str;
	int n_left;

	// get the board from the server, reading BUFSIZ at a time
	int n_read = recv(sfd, server_msg, BUFSIZ, 0); 
	if (n_read < 0) {
		std::cerr << "Error: recv failed." << std::endl;
		close(sfd);
		return 1;
	}
	std::tie(start_str, n_left) = parse_message(server_msg, server_start_format, body_format, body_keyword_len);
	while (n_left != 0) {
		assert(n_left > 0);
		n_read = recv(sfd, server_msg, std::min(BUFSIZ, n_left), 0);

		start_str.append(server_msg, n_read);
		n_left -= n_read;
	}

	// convert to json to extract data
	json start_json = json::parse(start_str);

	// Initialize the MVC framework
	Controller controller_(start_json["board"], start_json["pid"]);

	// READER THREAD: handle changelog messages from server, controller handles events
	std::thread changelog_t(handle_changelog, sfd, &controller_);

	// WRITER THREAD: handle user requests + send to server
	handle_user_input(sfd, &controller_);
	
	// Rejoin and close the file descriptors
	changelog_t.join();
	shutdown(sfd, SHUT_RDWR);
	close(sfd);
	
	return 0;
}