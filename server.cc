#include <stdio.h>
#include <iostream>
#include <thread>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "helpers.hh"
#include "game.hh"
#include "nlohmann/json.hpp"

#define PORT 6169
#define BOARD_SIZE 4
#define MAX_CONNECTIONS 5

using json = nlohmann::json;

// write 

bool write_to_socket(int cfd, std::string msg) {
    std::cout << "writing to cfd " << std::to_string(cfd) << std::endl;
    int n_written = write(cfd, msg.c_str(), msg.size());

    if (n_written < msg.size()) {
        fprintf(stderr, "Error writing to client on cdf: %d\n\n", cfd);
    }

    return n_written == msg.size();
}

// handle_request(request) 
//      Handle request from the client
//      Implements the API

void handle_request(json request, int cfd, int player_id, Game* g_ptr) {
    std::cout << request.dump() << std::endl;

    std::string response;

    // check to see if a move request
    if (request.find("move") != request.end()) {
        
        std::string dir = request["move"];
        // TODO handle typerrors --- should be wrapped in try catch
        // Currently assumes json will be well formed i.e. not an object or something 

        // parse command + send to game API
        if (dir == "up"){
            g_ptr->move_player(player_id, UP);
            response = "MOVE: up";
        } else if (dir == "up") {
            g_ptr->move_player(player_id, DOWN);
            response = "MOVE: down";
        } else if (dir == "down") {
            g_ptr->move_player(player_id, LEFT);
            response = "MOVE: left";
        } else if (dir == "right") {
            g_ptr->move_player(player_id, RIGHT);
            response = "MOVE: right";
        } else {
            response = "MOVE: invalid";
        }
    } else {
       response = "ERROR: invalid command";
    }

    std::cout << response << std::endl;
    write_to_socket(cfd, response);
    g_ptr->print_board();
    std::cout << std::endl;
}

// void handle_connection(cfd, player_id, g_ptr)
//      Reads from the socket for any client requests
//      Sends client events happening in the game 

void handle_connection(int cfd, int player_id, Game* g_ptr) {

    char buf[BUFSIZ];
    char* buf_ptr = buf;
    int len, nread, body_start_ind, body_len_in_buf;
    bool skip_get = false;
    char equals_sign;
    std::string request;

    g_ptr->print_board();
    std::cout << std::endl;

    // read the client request from the file
    //      skip_get == true when we know there is already a well formed header already in buf
    while (skip_get || recv(cfd, buf_ptr, BUFSIZ - (buf_ptr - buf), 0) > 0) {    

        // parse the request of form REQUEST len=XXX, body=YYY      
        if (sscanf(buf_ptr, "REQUEST len=%d, body%c", &len, &equals_sign) == 2 && equals_sign == '=') {
            
            // extract the REQUEST body from the string
            request = buf_ptr;
            nread = request.size();           
            body_start_ind = request.find("body=") + 5;    
            body_len_in_buf = std::min(len, nread - body_start_ind);
            request = request.substr(body_start_ind, body_len_in_buf);

            // IF (buffer contained entire request), handle request + loop back to handle next request
            if (len == body_len_in_buf) {
                handle_request(json::parse(request), cfd, player_id, g_ptr);
                
                // move the buffer pointer over to the start of the next request, update skip flag
                buf_ptr += body_start_ind + len;
                skip_get = (sscanf(buf_ptr, "REQUEST len=%d, body%c", &len, &equals_sign) == 2 && equals_sign == '=');

            // IF (buffer did not contain the entire request), read until we have the whole thing
            } else {
                buf_ptr = buf;
                skip_get = false;
                while (request.size() < len) {
                    // read from the stream, handling errors
                    int sz = len - request.size() < BUFSIZ ? len - request.size() : BUFSIZ;
                    if (recv(cfd, buf_ptr, BUFSIZ - (buf_ptr - buf), 0) < 0) {
                        close(cfd);
                        return;
                    } 
                    request += buf_ptr;   
                    
                    // TDD: if we read less than the buffer size, we must be done
                    if (strlen(buf_ptr) < BUFSIZ) {
                        assert(request.size() == len);
                    }
                }
            }

        // IF the request does not conform to the API, return some type of error
        } else {
            // TODO: return some type of error
        }
    }
    close(cfd);
}


int main(int argc, char** argv) {

    // create socket, bind, and listen 
    int sfd = init_socket(PORT, MAX_CONNECTIONS);
    if (sfd == -1) {
        printf("Could not init socket...shutting down\n");
        return 1;
    } else {
        printf("Listening on port %d...\n", PORT);
    }

    // create game obj, which will hold the shared state
    Game game_(BOARD_SIZE);

    // accept client connection, blocking while waiting for connection
    socklen_t c = sizeof(struct sockaddr);
    struct sockaddr_in client;

    int i = 0;
    while(true) {
        int cfd = accept(sfd, (sockaddr*) &client, &c);
        if (cfd < 0) {
            perror("accept");
            return 1;
        }

        printf("Client IP address: %s\n", inet_ntoa(client.sin_addr));  
        printf("Client Port: %d\n", (int) ntohs(client.sin_port));
        std::cout << "new cfd = " << cfd << std::endl;

        int player_id = game_.create_player(cfd);
        std::thread t(handle_connection, cfd, player_id, &game_);
        t.detach();
    } 

    close(sfd);

	return 0;
}