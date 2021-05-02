#include "client_helpers.hh"

using json = nlohmann::json;

// void handle user(int sfd)
//		handles

void handle_user(int sfd, Controller* c_ptr) { 	

	// get move from the user
	char key = c_ptr->get_next_move();
	while (key != 'q') {
		
		// format message and send to server
		std::string msg = format_client_move_request(key);
		if (send(sfd, msg.c_str(), msg.size(), 0) < 0)	{
			std::cerr << "Send failed" << std::endl;
			return;
		}

		key = c_ptr->get_next_move();
	}

	return;
}

// void handle_changelog(int sfd, Controller* c_ptr)
//		subscribes to the changelog by reading from the socket
//		parses the message and sends to the event handler
//		TODO: update to santize the raw server message

void handle_changelog(int sfd, Controller* c_ptr) {
	
	// loop, listening to the server for changelog updates
	char server_msg[BUFSIZ];

	// TOOD: update this to have some atomic that user is still connected
	while(true) {

		// read the message from the server
		// TODO: handle case where socket is already closed
		if (recv(sfd, server_msg, BUFSIZ, 0) <= 0) { 
			std::cerr << "ERROR: Read from server." << std::endl;
			close(sfd);
			return;
		}
		
		// parse the message from the server and handle the event
		// TODO: handle bad parse
		std::string event_str = parse_message(server_msg, server_event_format, body_format, body_keyword_len);
		if(!handle_event(json::parse(event_str), c_ptr)) {
			std::cerr << "ERROR: Event invalid." << std::endl;
			return;
		}
	}
}

// bool handle_event(json event_json, Controller* c_ptr)
//		handles the events from the raw json format, calling underlying board
//		currently 2 types of events:
//			(A) move --- moves the player in the board
//			(B) add --- adds a new player to the board

bool handle_event(json event_json, Controller* c_ptr) {

	if (event_json.find("move") != event_json.end()) {
		c_ptr->handle_event_move(event_json["move"]["pid"], event_json["move"]["dir"]);
		return true;
	} 

	if (event_json.find("add") != event_json.end()) {
		c_ptr->handle_event_add(event_json["add"]["pid"], event_json["add"]["loc"]);
		return true;
	}

	return false;
}

// int init socket
//      initializes and connects to socket
//      returns a socket fd (if == -1, there was an error)

int init_socket() {
    int sfd;
	struct sockaddr_in server;
	
	// create socket
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1) {
		std::cerr << "ERROR:Create socket failed." << std::endl;
		return sfd;
	}
	
	server.sin_addr.s_addr = inet_addr(path);
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	// connect to remote server
	if (connect(sfd, (struct sockaddr*) &server, sizeof(server)) < 0) {
		std::cerr << "ERROR:Connect failed." << std::endl;
        close(sfd);
		return -1;
	}

    return sfd;
}

// std::string format_client request(char move)
//      wraps the client request in the API wrapper
//      will be of form REQUEST len=xxx, body=abcdef...

std::string format_client_move_request(char move) {
	json request;	
	request["move"] = move;

    // extract the body into string form
    std::string body_str = request.dump();

    // create the message to send
    std::string msg = request_header;               // REQUEST len=
    msg.append(std::to_string(body_str.size()));    // REQUEST len=xx
    msg.append(body_header);                        // REQUEST len=xx, body=
    msg.append(body_str);                           // REQUEST len=xx, body={"move":move}

    return msg;
}