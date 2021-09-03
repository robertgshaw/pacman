#include <iostream>
#include <fstream>
#include <string>
#include <sys/socket.h> // socket
#include <arpa/inet.h>	// inet_addr
#include <unistd.h>
#include "../shared/nlohmann/json.hpp"

#include "../shared/utilities.hh"
#include "controller.hh"

// This file contains code which implements some helpers to handle the APIs
//      used to communicate between the client and the server

static const char* server_start_format = "START len=%d, body%c";
static const char* start_header = "START len=";

static const char* server_event_format = "EVENT len=%d, body%c";
static const char* event_header = "EVENT len=";

static const char* request_header = "REQUEST len=";

static const int body_keyword_len = 5; 
static const char* body_format = "body=";
static const char* body_header =", body=";

void handle_user_input(int sfd, Controller* c_ptr);
void handle_changelog(int sfd, Controller* c_ptr);
bool handle_event(nlohmann::json event_json, Controller* c_ptr);

// int init socket
//      wrapper around C socket syscalls
//      initializes and connects to socket
//      returns a sfd (if < 0, there was an error)
int init_socket();

// std::String add_request_wrapper(std::string body_str)
//		wrapes the body of the message in the correcet protocol
//		REQUEST len=xxx body=body_str

std::string add_request_wrapper(std::string body_str);

// std::string format_client_move_request(char move);
//      wraps the client request in the API wrapper
//      will be of form REQUEST len=xxx, body={"move":move}

std::string format_client_move_request(char move);

// std::string format_client_quit_request();
//      wraps the client request in the API wrapper
//      will be of form REQUEST len=xxx, body={"quit":1}

std::string format_client_quit_request();