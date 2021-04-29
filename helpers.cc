#include "helpers.hh"
using json = nlohmann::json;

// This file contains code which implements the communication API
// as well as some basic wrappers around SYSCALL utilities 

/*
 *
 ***** Handles API Calls *****
 *
 */

// API defined as follows
//      Clients send requests of form REQUEST len=[int], body=[command]
//          List of client commands described below (handle request function)
//
//      Server sends [TBU]

// constants that define the API
static const char* request_format = "REQUEST len=%d, body%c";
static const char* body_request_format = "body=";
static const int body_keyword_len = 5; // "body=" is 5 chars

// void handle_connection(cfd, player_id, g_ptr)
//      MAIN LOOP EXECUTED BY THE THREAD, does two things
//          1)  Reads from the socket, looking for client requests,
//              + handling the commands in the requests
//
//          2)  Sends events that are happening to the client,
//              while listening to the changelog

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
        if (sscanf(buf_ptr, request_format, &len, &equals_sign) == 2 && equals_sign == '=') {
            
            // extract the REQUEST body from the string
            request = buf_ptr;
            nread = request.size();           
            body_start_ind = request.find(body_request_format) + body_keyword_len;    
            body_len_in_buf = std::min(len, nread - body_start_ind);
            request = request.substr(body_start_ind, body_len_in_buf);

            // IF (buffer contained entire request), handle request + loop back to handle next request
            if (len == body_len_in_buf) {
                handle_request(json::parse(request), cfd, player_id, g_ptr);
                
                // move the buffer pointer over to the start of the next request, update skip flag
                buf_ptr += body_start_ind + len;
                skip_get = (sscanf(buf_ptr, request_format, &len, &equals_sign) == 2 && equals_sign == '=');

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

// handle_request(request) 
//      IMPLEMENTS INTERFACE BETWEEN CLIENT CONNECTION + BOARD API
//      Command options include
//          1)  "move":"dir" --- dir = up, down, left, right (moves the player on the board)

void handle_request(json request, int cfd, int player_id, Game* g_ptr) {
    std::cout << request.dump() << std::endl;

    std::string response;

    // check to see if a move command was passed
    if (request.find("move") != request.end()) {
        
        std::string dir = request["move"];
        // TODO handle typerrors --- should be wrapped in try catch
        // Currently assumes json will be well formed i.e. not an object or something 

        // parse command + send to game API
        if (dir == "up"){
            g_ptr->move_player(player_id, UP);
            response = "MOVE: up";
        } else if (dir == "down") {
            g_ptr->move_player(player_id, DOWN);
            response = "MOVE: down";
        } else if (dir == "left") {
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
    
    // send response to the client
    write_to_socket(cfd, response);
    
    // print board out for the time being to observe state
    g_ptr->print_board();
    std::cout << std::endl;
}

/* 
 *
 ***** WRAPPERS AROuND SYSCALLS *****
 *
 */

// write_to_socket(cfd, msg)
//      wrapper around underlying c syscalls
//      returns true if successfully wrote entire message

bool write_to_socket(int cfd, std::string msg) {
    // 
    int n_written = write(cfd, msg.c_str(), msg.size());
    if (n_written < msg.size()) {
        std::cerr << "Error writing to client on cdf" << std::to_string(cfd) << std::endl;
    }

    return n_written == msg.size();
}

// init_socket()
//      connect, binds, listens 
//      returns a socket fd; returns -1 if there was an error

int init_socket(int port, int max_conns) {
    
    // create
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1) {
        printf("Could not create socket\n");
        return -1;
    } 

    // bind
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    if (bind(sfd, (sockaddr*) &server, sizeof(server)) < 0) {
        perror("bind");
        return -1;
    }

    // listen
    if (listen(sfd, max_conns) < 0) {
        perror("listen");
        return -1;
    }

    return sfd;
}

