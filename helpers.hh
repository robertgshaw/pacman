#ifndef HELPERS_H
#define HELPERS_H

#include <iostream>
#include <memory>
#include <thread>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "nlohmann/json.hpp"

#include "game.hh"

// This file contains code which implements the API communicated across the socket
//      as well as some basic wrappers around SYSCALL utilities 
//  

//
//
// ********** CLIENT SERVER API **********
//
//

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
static const char* board_header = "BOARD len=";
static const char* board_body_header =", body=";

// constants that define the client to server API
static const char* client_request_format = "REQUEST len=%d, body%c";
static const char* client_body_request_format = "body=";
static const int client_body_keyword_len = 5; 


// void handle_player(cfd, player_id, g_ptr, board_json)
//      (1) Initializes client by sending across the board_json as is
//      (2) Spins up new thread to listen for the changelog (part B of API)
//      (3) Uses this thread to listen for client commands (part A of API)

void handle_player(int cfd, int player_id, Game* g_ptr, nlohmann::json board_json);

//
// (1) CLIENT REQUESTS
//

// void handle_connection(cfd, player_id, g_ptr)
//      MAIN LOOP EXECUTED BY THE THREAD, handling client requests:
//          A)  Reads from the socket, looking for client requests,
//              + handling the commands in the requests

void handle_connection(int cfd, int player_id, Game* g_ptr);

// handle_request(request) 
//      IMPLEMENTS INTERFACE BETWEEN CLIENT CONNECTION + BOARD API
//      Command options include:
//          A)  "move":"dir" --- dir = up, down, left, right (moves the player on the board)

void handle_request(nlohmann::json request, int cfd, int player_id, Game* g_ptr);

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

// write_to_socket(cfd, msg)
//      wrapper around underlying c syscalls
//      returns true if successfully wrote entire message

bool write_to_socket(int cfd, std::string msg);

// int init_socket(port, max_conns)
//      connect, binds, listens 
//      binds to port
//      limits max_conns queue length in listen call
//      returns a socket fd; returns -1 if there was an error

int init_socket(int port, int max_conns);


#endif