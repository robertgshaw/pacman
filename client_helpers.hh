#include <iostream>
#include <string>
#include <sys/socket.h> // socket
#include <arpa/inet.h>	// inet_addr
#include <unistd.h>
#include "nlohmann/json.hpp"

#include "utilities.hh"
#include "board.hh"

// This file contains code which implements some helpers to handle the APIs
//      used to communicate between the client and the server

static const char* server_board_format = "BOARD len=%d, body%c";
static const char* board_header = "BOARD len=";

static const char* server_event_format = "EVENT len=%d, body%c";
static const char* event_header = "EVENT len=";

static const char* request_header = "REQUEST len=";

static const int body_keyword_len = 5; 
static const char* body_format = "body=";
static const char* body_header =", body=";

static const int port = 6169;
static const char* path = "127.0.0.1";

void handle_user(int sfd);
void handle_changelog(int sfd, Board* b_ptr);
bool handle_event(nlohmann::json event_json, Board* b_ptr);

// int init socket
//      wrapper around C socket syscalls
//      initializes and connects to socket
//      returns a sfd (if < 0, there was an error)
int init_socket();

// std::string format_client request(json request)
//      wraps the client request in the API wrapper
//      will be of form REQUEST len=xxx, body=abcdef...

std::string format_client_request(nlohmann::json request);