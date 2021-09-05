#include "client_api.hh"

using json = nlohmann::json;

// void handle_user_input(int sfd)
//		listens to the user at the commandline and handles the user input
//		sending messages to the server depending on the key passed back
//		note: think more about syncrhonizing the set quit shared boolean
// 			think we are okay to not synchronize, since acceptable for the changelog thread
//			to continue listening (since we know another request will eventually come)
//			

void handle_user_input(int sfd, Controller* c_ptr) { 	
	std::ofstream ofs;
	ofs.open("user_input.txt", std::ofstream::out | std::ofstream::app);

	// get move from the user
	char key = c_ptr->get_next_move();

	// while the user has yet to quit
	while (key != 'q' && !c_ptr->should_quit()) {
		
		// format message and send to server
		std::string msg = format_client_move_request(key);
		ofs << msg.c_str() << std::endl;

		if (send(sfd, msg.c_str(), msg.size(), 0) < 0)	{
			c_ptr->set_quit();
			ofs << "Send move request failed" << std::endl;
			return;
		}

		// listen for the next move
		key = c_ptr->get_next_move();
	}

	// if the user sent "q", send a quit request
	c_ptr->set_quit();
	std::string msg = format_client_quit_request();
	ofs << msg << std::endl;

	if (send(sfd, msg.c_str(), msg.size(), 0) < 0)	{
		ofs << "Send quit request failed" << std::endl;
		return;
	}

	ofs.close();
	return;
}

// void handle_changelog(int sfd, Controller* c_ptr)
//		subscribes to the changelog by reading from the socket
//		parses the message and sends to the event handler
//		TODO: update to santize the raw server message

void handle_changelog(int sfd, Controller* c_ptr) {

	std::ofstream ofs;
	ofs.open("changelog_output.txt", std::ofstream::out | std::ofstream::app);

	// loop, listening to the server for changelog updates
	char server_msg[BUFSIZ];

	// TOOD: update this to have some atomic that user is still connected
	while(true) {

		// read the message from the server
		if (recv(sfd, server_msg, BUFSIZ, 0) <= 0) {
			c_ptr->set_quit(); 
			ofs << "ERROR: Read from server." << std::endl;
			ofs.close();
			return;
		}
		
		// check if the UI has suggested that we should quit before
		// if it has, then we will eventually get here (since user quitting is an event)
		if (c_ptr->should_quit()) {
			ofs.close();
			return;
		}
		
		// parse the message from the server and handle the event
		// TODO: handle bad parse
		std::string event_str;
		int n_left;
		std::tie(event_str, n_left) = parse_message(server_msg, server_event_format, body_format, body_keyword_len);
		
		ofs << event_str << std::endl;

		if(!handle_event(json::parse(event_str), c_ptr)) {
			c_ptr->set_quit(); 
			ofs << "ERROR: Event invalid." << std::endl;
			ofs.close();
			return;
		}
	}
}

// bool handle_event(json event_json, Controller* c_ptr)
//		handles the events from the raw json format, passing to the controller:
//			(A) move:	moves the player in the board
//			(B) add:	adds a new player to the board
//			(C) quit:	removes a player from the board

bool handle_event(json event_json, Controller* c_ptr) {

	// (a) handle move
	if (event_json.find("move") != event_json.end()) {
		c_ptr->handle_event_move(event_json["move"]["pid"], event_json["move"]["dir"]);
		return true;
	} 

	// (B) handle add
	if (event_json.find("add") != event_json.end()) {
		c_ptr->handle_event_add(event_json["add"]["pid"], event_json["add"]["loc"]);
		return true;
	}

	// (C) handle quit
	if (event_json.find("quit") != event_json.end()) {
		c_ptr->handle_event_quit(event_json["quit"]["pid"], event_json["quit"]["loc"]);
		return true;
	}

	return false;
}

// 												  						//
//												  						//
// 	********** WRAPPERS AROUND SYSCALLS, PROTOCOL UTILITIES **********	//
//												  						//
//												  						//

// int init socket
//      initializes and connects to socket
//      returns a socket fd (if == -1, there was an error)

int init_socket(const int port, const char* path) {
    int sfd;
	struct sockaddr_in server;
	
	// create socket
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1) {
		std::cerr << "ERROR: Create socket failed." << std::endl;
		return sfd;
	}
	
	server.sin_addr.s_addr = inet_addr(path);
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	// connect to remote server
	if (connect(sfd, (struct sockaddr*) &server, sizeof(server)) < 0) {
		std::cerr << "ERROR: Connect failed." << std::endl;
        close(sfd);
		return -1;
	}

    return sfd;
}

// std::String add_request_wrapper(std::string body_str)
//		wrapes the body of the message in the correcet protocol
//		REQUEST len=xxx body=body_str

std::string add_request_wrapper(std::string body_str) {
	// create the message to send
    std::string msg = request_header;               // REQUEST len=
    msg.append(std::to_string(body_str.size()));    // REQUEST len=xx
    msg.append(body_header);                        // REQUEST len=xx, body=
    msg.append(body_str);                           // REQUEST len=xx, body={"request":request}

	return msg;
}

// std::string format_client_request_move(char move)
//      wraps the client request in the API wrapper
//      will be of form REQUEST len=xxx, body={"move":dir}

std::string format_client_move_request(char move) {
	json request;	
	request["move"] = move;
    return add_request_wrapper(request.dump());
}

// std::string format_client_quit(char move)
//      wraps the client request in the API wrapper
//      will be of form REQUEST len=xx, body={"quit":1}

std::string format_client_quit_request() {
	json request;	
	request["quit"] = 1;
    return add_request_wrapper(request.dump());
}