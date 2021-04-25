#ifndef HELPERS_H
#define HELPERS_H

#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

// int init_socket(port, max_conns)
//      connect, binds, listens 
//      binds to port
//      limits max_conns queue length in listen call
//      returns a socket fd; returns -1 if there was an error

int init_socket(int port, int max_conns);

// std:string format_msg(body)
//      adds useful headers to the msg
//      message length, others potentially to be added

std::string format_msg(json body);

#endif