#ifndef SERVER_API_H
#define SERVER_API_H

#include <iostream>
#include <memory>
#include <thread>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h> // FD_SET, FD_ISSET, FD_ZERO macros
#include "../shared/nlohmann/json.hpp"

#include "game.hh"
#include "exitpipe.hh"

// This file contains code which implements the API communicated across the network
//      as well as some basic wrappers around SYSCALL utilities 

//
// CONSTANTS for the game
//

static const int port = 5000;
static const int board_size = 21;
static const int max_players = 5;

//
//
// ********** CLIENT SERVER API **********
//
//

// Server design as follows:
//      Event driven architecture - each client is responsible for maintaining its own local state of the game
//          The clients connect to the server. The server then sends clients any "event" that happens in the game/
//          As such, the server's job is to (1) recieve requests from the client (2) send out events to the client
//
//      Utilize a reader thread / writer thread paradigm to handle getting requests and sending out events
//          Since sockets are fully duplex and we use a single reader thread + a single writer thread,
//          there is a no race condition + therefore no synchronization needed for reads / writes.
// 
//      Utilize an "exit pipe" to notify threads to exit if called by the server's command line
//          Each connection is passed a pipe fd (exit_pipe_fd) which can recieve this message from the main thread

// API defined as follows:
//      (A) Clients send requests containing commands to the server
//          Command are of form     REQUEST len=[int], body=[request]
//
//      (B) Server listens to the changelog; when events pushed sends to client
//          Events are of form      EVENT len=[int], body=[event]

// API defined as follows:
//      (A) Clients send requests containing commands to the server
//          Command are of form     REQUEST len=[int], body=[request]
//
//      (B) Server listens to the changelog; when events pushed sends to client
//          Events are of form      EVENT len=[int], body=[event]

//
// CONSTANTS for the API
//

// constants that define the server to client API
static const char* event_header = "EVENT len=";
static const char* event_body_header =", body=";
static const char* start_header = "START len=";
static const char* start_body_header =", body=";

// constants that define the client to server API
static const char* client_request_format = "REQUEST len=%d, body%c";
static const char* client_body_request_format = "body=";
static const int client_body_keyword_len = 5; 

// constants that enable server side "exit" / "quit" requests on errors
static const nlohmann::json exit_request = {{ "exit" , 1 }};
static const nlohmann::json quit_request = {{ "quit" , 1 }}; 

// void run_server(exit_pipe_fd)
//      MAIN thread that handles an active game
//      Listens for new client connections with accept() - once accepted, spins off a new connection
//      Listens to the "exit pipe" simulataneously (with "select") - if signal, shutdown the connections + cleanup

void run_server(int exit_pipe_fd);

// void handle_connection(cfd, exit_pipe_cfd, player_id, g_ptr, board_json)
//      (1) Initializes client by sending across the board_json as is
//      (2) Spins up new thread to listen for the changelog (part B of API)
//      (3) Uses this thread to listen for client commands (part A of API)

void handle_connection(int cfd, int exit_pipe_cfd, int player_id, Game* g_ptr, nlohmann::json board_json);

//
// (1) CLIENT REQUESTS
//

// void handle_requests(cfd, exit_pipe_cfd, player_id, g_ptr)
//      MAIN LOOP EXECUTED BY THE THREAD, handling client requests:
//          (A) Blocks on client socket and on exit_pipe_cfd, reading + processing, until:
//              (1) there is a signal from the exit_pipe, telling us to shutdown
//              (2) there is a quit request sent by the client
//              (3) some type of error occurs with the socket

void handle_requests(int cfd, int exit_pipe_fd, int player_id, Game* g_ptr);

// handle_request(request) 
//      IMPLEMENTS INTERFACE BETWEEN CLIENT CONNECTION + BOARD API
//      Command options include:
//          A)  "move":"dir"    --- moves the player on the board dir = up, down, left, right
//          B)  "quit":1        --- removes the player from the board           
//
//      Returns true if the command was quit to alert caller of need to close down

bool handle_request(nlohmann::json request, int cfd, int player_id, Game* g_ptr);

// parse_request(buf_ptr, request)
//      parses the request according to the API from the buffer
//      buf_ptr, request are passed by REFERENCE + state is updated
//          returns true if there was a request to process
//          returns false if there was a non-conforming request

bool parse_request(char*& buf_ptr, std::string& request);

//
// (2) CHANGELOG EVENTS
//

// void handle_changelog(cfd, player_id, g_ptr)
//      MAIN LOOP EXECUTED BY THE THREAD, subscribes to the changelog
//          A)  Listens to events in the changelog, sending them to the client side

void handle_changelog(int cfd, int player_id, Game* g_ptr);

// std::string format_sever_msg(json command, char* header, char* body_header)
//      wraps the client request in the API wrapper
//      will be of form COMMAND len=xxx, body=[command]

std::string format_server_msg(nlohmann::json command, const char* header, const char* body_header);

//
//
// ********** SYSCALL WRAPPERS **********
//
//

// is_valid_request_header(buf_ptr, len)
//      checks whether a request header is well formatted
//      updates len with the lenght of the request

bool is_valid_request_header(char* buf_ptr, int& len);

// is_empty_buf(ptr)
//      checks if a buffer is empty or has data to read

bool is_empty_buf(char* ptr);


// write_to_socket(cfd, msg)
//      wrapper around underlying c syscalls
//      returns true if successfully wrote entire message

bool write_to_socket(int cfd, std::string msg);

// read_from_socket(cfd, buf_ptr, sz):
//      reads from socket, blocking
//      returns true if there is text to read from buf_ptr
//      returns false if there is none (i.e. socket connection closed)

bool read_from_socket(int cfd, char* buf_ptr, int sz);

// int init_socket(port, max_conns)
//      connect, binds, listens 
//      binds to port
//      limits max_conns queue length in listen call
//      returns a socket fd; returns -1 if there was an error

int init_socket(int port, int max_conns);

#endif