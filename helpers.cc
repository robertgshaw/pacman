#include "helpers.hh"
#include "event.hh"

// rename this class server_api.cc (handles the server side of the client server API)

using json = nlohmann::json;

// This file supports server.cc and contains:
//      1) Client-server communication protocol API
//      2) Wrappers Around Basic Socket Syscalls 

//
//
// ********** CLIENT SERVER API IMPLEMENTATION **********
//
//

// Server design as follows:
//      Utilize a reader thread / writer thread paradigm
//          Since sockets are fully duplex and we use a single reader thread + a single writer thread,
//          there is a no race condition + therefore no synchronization needed for reads / writes
// 
//          However, we need to synchronize closing the cfd, as race conditions can exist +  
//          calling close on already closed fd can cause undefined behavior. Interesting article on options here:
//              https://www.drdobbs.com/parallel/file-descriptors-and-multithreaded-progr/212001285    

// API defined as follows:
//      (A) Clients send requests containing commands to the server
//          Command are of form     REQUEST len=[int], body=[request]
//
//      (B) Server listens to the changelog; when events pushed sends to client
//          Events are of form      EVENT len=[int], body=[event]


// void handle_connection(cfd, player_id, g_ptr, board_json)
//      Does the following:
//          (1) Initializes client by sending across the board_json as is
//          (2) Spins up new thread to listen for the changelog (part B of API)
//          (3) Uses this thread to listen for client commands (part A of API)
//          (4) Rejoins together and frees the OS resources

void handle_connection(int cfd, int player_id, Game* g_ptr, json board_json) {
    
    // (1) send initial board to the socket
    if (!write_to_socket(cfd, format_server_msg(board_json, board_header, board_body_header))) {
        std::cerr << "Failed to send board for p_id:" << std::to_string(player_id) << std::endl;
        close(cfd);
        return;
    }

    // (2) spin up thread to listen to changelog - writes changes to socket
    std::thread t(handle_changelog, cfd, player_id, g_ptr);
    
    // (3) use this thread to listen to for user commands - reader from socket
    handle_requests(cfd, player_id, g_ptr);
    
    // (4) join together and free underlying resources
    t.join();
    shutdown(cfd, SHUT_RDWR);
    if(close(cfd) < 0) {
        std::cout << "cLose failed" << std::endl;
    }

    return;

}


// (A) CLIENT REQUESTS

// void handle_requests(cfd, player_id, g_ptr)
//      MAIN LOOP EXECUTED BY THE THREAD, handling client requests:
//          1)  Reads from the socket, looking for client requests,
//              + handling the commands in the requests

void handle_requests(int cfd, int player_id, Game* g_ptr) {

    char buf[BUFSIZ];
    memset(buf, '\0', BUFSIZ); // clears out memory
    char* buf_ptr = buf;
    int len, nread, body_start_ind, body_len_in_buf;
    bool skip_get = false;
    bool is_quit;
    char equals_sign;
    std::string request;

    g_ptr->print_board();
    std::cout << std::endl;
   
    // read the client request from the file
    //      skip_get == true when we know there is already a well formed header already in buf
    while (skip_get || recv(cfd, buf_ptr, BUFSIZ - (buf_ptr - buf), 0) > 0) {    

        // parse the request of form REQUEST len=XXX, body=YYY      
        if (sscanf(buf_ptr, client_request_format, &len, &equals_sign) == 2 && equals_sign == '=') {
            
            // extract the REQUEST body from the string
            request = buf_ptr;
            nread = request.size();           
            body_start_ind = request.find(client_body_request_format) + client_body_keyword_len;    
            body_len_in_buf = std::min(len, nread - body_start_ind);
            request = request.substr(body_start_ind, body_len_in_buf);

            // IF (buffer contained entire request), handle request + loop back to handle next request
            if (len == body_len_in_buf) {
                
                // handle the json request; if true, then the request was to quit 
                // as such, return from this function and go back to the client connection thread to clean up the cfd
                if (handle_request(json::parse(request), cfd, player_id, g_ptr)) {
                    return;
                }
                
                // move the buffer pointer over to the start of the next request, update skip flag
                buf_ptr += body_start_ind + len;
                skip_get = (sscanf(buf_ptr, client_request_format, &len, &equals_sign) == 2 && equals_sign == '=');

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

// void handle_request(request, cfd, player_id, g_ptr) 
//      IMPLEMENTS INTERFACE BETWEEN CLIENT CONNECTION + BOARD API
//      Command options include
//          1)  "move":"dir" ---> dir = up, down, left, right (moves the player on the board)
//          2)  "quit":1     ---> 1 = value for protocol consistency (removes the player from the board) 

bool handle_request(json request, int cfd, int player_id, Game* g_ptr) {

    std::string response;
    bool is_quit = false;

    // (1) handle move
    if (request.find("move") != request.end()) {
        // TODO: handle badly formed json
        int dir = request["move"];
        if (dir == UP || dir == DOWN || dir == LEFT || dir == RIGHT) {
            g_ptr->handle_request_move(player_id, dir);
        }        
    }

    // (2) handle quit
    if (request.find("quit") != request.end()) {
        // TODO: update code so user knows its own player id
        g_ptr->handle_request_quit(player_id);
        is_quit = true;
	}
    
    // print board out for the time being to observe state
    g_ptr->print_board();
    std::cout << std::endl;

    return is_quit;
}


// (B) CHANGELOG EVENTS

// void handle_changelog(cfd, player_id, g_ptr)
//      MAIN LOOP EXECUTED BY THE THREAD, subscribes to the changelog
//          1)  Listens to events in the changelog, sending them to the client side

void handle_changelog(int cfd, int player_id, Game* g_ptr) {
    
    // loop continuously until the the socket is disconnected by the client
    bool is_connected = true;
    while(is_connected) {

        // get event from changelog (note: this blocks) + write to user
        json e_json = g_ptr->get_next_event(player_id);
        is_connected = write_to_socket(cfd, format_server_msg(e_json, event_header, event_body_header));

        // if it is a quit event, return (joining the changelog thread back to client connection thread)
        if (g_ptr->is_quit_event(player_id, e_json)) {
            return;
        }
    }

    return;
}

// 
//
// ********** SYSCALL WRAPPERS + PROTOCOL UTILITIES **********
//
//

/// std::string format_server_board(json board)
//      wraps the sever board in the API wrapper
//      will be of form BOARD len=[xxx], body=[board]...

std::string format_server_msg(nlohmann::json command, const char* header, const char* body_header) {
    // extract the body into string form
    std::string body_str = command.dump();

    // create the message to send
    std::string msg = header;                       // COMMAND len=
    msg.append(std::to_string(body_str.size()));    // COMMAND len=xxx
    msg.append(body_header);                        // COMMAND len=xxx, body=
    msg.append(body_str);                           // COMMAND len=xxx, body=abcdef...

    return msg;
}

// write_to_socket(cfd, msg)
//      wrapper around underlying c syscalls
//      returns true if successfully wrote entire message

bool write_to_socket(int cfd, std::string msg) {
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

