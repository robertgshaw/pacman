#ifndef HELPERS_H
#define HELPERS_H

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "nlohmann/json.hpp"
#include "game.hh"

// This file contains code which implements the API communicated across the socket
//      as well as some basic wrappers around SYSCALL utilities 
//  

/*
 *
 ***** API CALLS *****
 *
 */

// API defined as follows
//      Clients send requests of form REQUEST len=[int], body=[command]
//          List of client commands described below (handle request function)


// void handle_connection(cfd, player_id, g_ptr)
//      MAIN LOOP EXECUTED BY THE THREAD, does two things
//          1)  Reads from the socket, looking for client requests,
//              + handling the commands in the requests
//
//          2)  Sends events that are happening to the client,
//              while listening to the changelog

void handle_connection(int cfd, int player_id, Game* g_ptr);

// handle_request(request) 
//      IMPLEMENTS INTERFACE BETWEEN CLIENT CONNECTION + BOARD API
//      Command options include
//          1)  "move":"dir" --- dir = up, down, left, right (moves the player on the board)

void handle_request(json request, int cfd, int player_id, Game* g_ptr);

/* 
 *
 ***** WRAPPERS AROuND SYSCALLS *****
 *
 */

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