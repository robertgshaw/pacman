#include "server_api.hh"
#include "event.hh"

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

// void run_server(exit_pipe_fd)
//      MAIN thread that handles an active game
//      Listens for new client connections with accept() - once accepted, spins off a new connection
//      Listens to the "exit pipe" simulataneously (with "select") - if signal, shutdown the connections + cleanup

void run_server(int exit_pipe_fd) {

    // initialize socket
    std::cout << "Initializing socket ... ";    
    int sfd = init_socket(port, max_players);
    if (sfd == -1) {
        std::cout << "[FAILED] ... Shutting down\n" << std::endl;
        return;
    } else {
        std::cout << "[DONE]" << std::endl;
    }
    
    // initialize the game
    std::cout << "Initializing game ... ";

    // init game objects + connection objects
    Game game_(board_size);
    Exitpipe exitpipe_;                     
    std::vector<std::thread> connections;

    std::cout << "[DONE]" << std::endl;

    // vars for the main loop below
    struct sockaddr_in client;
    socklen_t c = sizeof(struct sockaddr);
    int player_id;
    json board_json;

    // vars for select
    fd_set readfds;
    int max_fd = sfd > exit_pipe_fd ? sfd : exit_pipe_fd;

    // accept client connection + spin off connection threads until exit
    bool is_exited = false;
    while(!is_exited) {

        // setup readfds set for select
        FD_ZERO(&readfds);
        FD_SET(exit_pipe_fd, &readfds);
        FD_SET(sfd, &readfds);

        // accept new connections / listen for exit_pipe
        int r = select(max_fd + 1, &readfds, NULL, NULL, NULL);                
        if (r < 0) {
            std::cerr << "TODO: shutdown the board on error" << std::endl;      // TODO: make this a clean exit
        } else if (r == 0) {
            std::cerr << "TODO: shutdown the board on timeout:" << std::endl;   // TODO: make this a clean exit

        // if there is a signal in the exit pipe
        } else if (FD_ISSET(exit_pipe_fd, &readfds)) {
            is_exited = true;

        // if there was a new socket connection
        } else {
            assert(FD_ISSET(sfd, &readfds));
            int cfd = accept(sfd, (sockaddr*) &client, &c);
            if (cfd < 0) {
                std::cerr << "Error: accept on sfd returned -1" << std::endl;   // TODO: make this a clean exit
                close(cfd);
                is_exited = true;
            }

            // add a new exit pipe pair
            exitpipe_.add_fd_pair();
            
            // create player + launch connection threads
            std::tie(player_id, board_json) = game_.handle_add_player(cfd);
            connections.push_back(std::thread(handle_connection, cfd, exitpipe_.pop_reader_fd(), player_id, &game_, board_json));
        }
    } 

    // send signal to the connection threads that it is time to close
    exitpipe_.exit_all();

    // join connection threads back together
    for (std::thread & connection : connections) {
        if (connection.joinable()) {
            connection.join();
        } else {
            // should not get here, since we never detach or join otherwise
            std::cerr << "Error: Thread " << connection.get_id() << " could not be joined" << std::endl;
            assert(0 == 1);
        }
    }

    // cleanup all the exit pipe resources
    exitpipe_.close_all();

    std::cout << "Closing sfd" << std::endl;
    // cleanup the socket
    shutdown(sfd, SHUT_RDWR);
    if(close(sfd) < 0) {
        std::cerr << "Error: close failed for sfd" << std::endl;
    }
}

// void handle_connection(cfd, player_id, g_ptr, board_json)
//      Does the following:
//          (1) Initializes client by sending across the board_json as is
//          (2) Spins up new thread to listen for the changelog (part B of API)
//          (3) Uses this thread to listen for client commands (part A of API)
//          (4) Rejoins together and frees the OS resources

void handle_connection(int cfd, int exit_pipe_fd, int player_id, Game* g_ptr, json board_json) {
    
    json start_json;
    start_json["pid"] = player_id;
    start_json["board"] = board_json;

    // (1) send initial board to the socket
    if (!write_to_socket(cfd, format_server_msg(start_json, start_header, start_body_header))) {
        std::cerr << "Error: write to socket failed for p_id:" << std::to_string(player_id) << std::endl;
        shutdown(cfd, SHUT_RDWR);
        if(close(cfd) < 0) {
            std::cerr << "Error: Close failed for player_id:" << std::to_string(player_id) << " after failed write to socket" << std::endl;
        }
        return;
    }

    std::cout << "Opened connection for player_id: " << std::to_string(player_id) << std::endl;

    // (2) spin up thread to listen to changelog - writer to socket
    std::thread t(handle_changelog, cfd, player_id, g_ptr);
    
    // (3) use this thread to listen to for user commands - reader from socket
    handle_requests(cfd, exit_pipe_fd, player_id, g_ptr);
    
    // (4) join together and free underlying resources
    t.join();
    shutdown(cfd, SHUT_RDWR);
    if(close(cfd) < 0) {
        std::cerr << "Error: close failed for player_id:" << std::to_string(player_id) << " on cfd:" << std::to_string(cfd) << std::endl;
    }
    
    std::cout << "Closed connection for player_id: " << std::to_string(player_id) << std::endl;
    return;
}

// (A) CLIENT REQUESTS

// void handle_requests(cfd, exit_pipe_cfd, player_id, g_ptr)
//      MAIN LOOP EXECUTED BY THE THREAD, handling client requests:
//      Blocks on client socket and on exit_pipe_cfd
//      Reads + processes requests, publishing to the changelog until:
//          (1) there is a signal from the exit_pipe, telling us to shutdown
//          (2) there is a quit request sent by the client
//          (3) some type of error occurs with the socket

void handle_requests(int cfd, int exit_pipe_fd, int player_id, Game* g_ptr) {
    
    // buffer to be filled by the socket reads
    char buf[BUFSIZ];
    memset(buf, '\0', BUFSIZ);
    char* buf_ptr = buf;

    // request to be extracted in the loop
    std::string request;    

    // loop, handling requests until exit
    bool is_exited = false;
    while (!is_exited) {

        // if there is data in the buffer waiting, parse request + handle if conforming API call
        if (!is_empty_buf(buf_ptr)) {
            
            // parse and handle request, exiting if a quit or an exit command
            if (parse_request(buf_ptr, request)) {
                is_exited = handle_request(json::parse(request), cfd, player_id, g_ptr);
            } else {
                is_exited = handle_request(quit_request, cfd, player_id, g_ptr);
                assert(is_exited == true);
            }

        // if there is no data in the buffer, check to see if a new message / exit command
        } else {
            // clear out buffer
            memset(buf, '\0', buf_ptr - buf);
            buf_ptr = buf;

            // reset select fd_sets
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(exit_pipe_fd, &readfds);
            FD_SET(cfd, &readfds);
            
            // wait until either there is data from the cfd socket OR from the exit signal pipe
            int r = select((cfd > exit_pipe_fd ? cfd : exit_pipe_fd) + 1, &readfds, NULL, NULL, NULL);
            if (r < 0) {
                std::cerr << "Error: select in handle_requests returned an error" << std::endl;
                is_exited = true;
            } else if (r = 0) {
                std::cerr << "Error: select in handle_request timed out" << std::endl;
                is_exited = true;
            } else {

                // if we got a signal in the exit_pipe_cfd, publish an exit request to changelog and exit
                if (FD_ISSET(exit_pipe_fd, &readfds)) {
                    is_exited = handle_request(exit_request, cfd, player_id, g_ptr);
                    assert(is_exited == true);
                
                // if we got a signal from the socket, read from socket into the buffer, exiting if there is nothing / an error
                } else {
                    assert(FD_ISSET(cfd, &readfds));
                    if (!read_from_socket(cfd, buf_ptr, BUFSIZ)) {
                        is_exited = handle_request(quit_request, cfd, player_id, g_ptr);
                        assert(is_exited == true);
                    }
                }
            }   
        }
    }

}

// void handle_request(request, cfd, player_id, g_ptr) 
//      IMPLEMENTS INTERFACE BETWEEN CLIENT CONNECTION + BOARD API
//      Command options include
//          1)  "move":"dir" ---> dir = up, down, left, right (moves the player on the board)
//          2)  "quit":1     ---> 1 = value for protocol consistency (removes the player from the board) 
//          3)  "exit":1     ---> 1 = value for protocol consistency (removes the player from the board)
//      Returns true if the request was a quit / exit command

bool handle_request(json request, int cfd, int player_id, Game* g_ptr) {

    bool is_quit = false;
    
    // (1) handle move
    if (request.find("move") != request.end()) {
        int dir = request["move"];
        if (dir == UP || dir == DOWN || dir == LEFT || dir == RIGHT) {
            g_ptr->handle_request_move(player_id, dir);
            
        }    
    }

    // (2) handle quit
    if (request.find("quit") != request.end()) {
        g_ptr->handle_request_quit(player_id);
        return true;
	}

    // (3) handle exit command
    if (request.find("exit") != request.end()) {
        g_ptr->handle_request_exit(player_id);
        return true;
	}

    return false;
}

// bool parse_request(buf_ptr, request)
//      parses the request according to the API from the buffer
//      buf_ptr, request are passed by REFERENCE + state is updated
//      returns true if there was a request to process
//      returns false if there was a non-conforming request

bool parse_request(char*& buf_ptr, std::string& request) {
    int len;

    // if the request header conforms to the API, attempt to extract request body
    if (is_valid_request_header(buf_ptr, len)) {
    
        // extract the REQUEST body from the buffer
        request = buf_ptr;
        int nread = request.size();           
        int body_start_ind = request.find(client_body_request_format) + client_body_keyword_len;    
        int body_len_in_buf = std::min(len, nread - body_start_ind);
        request = request.substr(body_start_ind, body_len_in_buf);

        // if buffer contained entire request, return, updating the state trackers
        if (body_len_in_buf == len) {
            
            // move the buffer pointer over to the start of the next request, update skip flag
            buf_ptr += body_start_ind + len;
            return true;

        } else {
            std::cerr << "Error: buffer could not fit the entire request for client" << std::endl;
            return false;
        }

    // if the request does not conform to the API, pass error
    } else {
        std::cerr << "Error: request does not conform to the API for client" << std::endl;
        return false;
    }

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

        // if it is a exit / quit event, return (joining the changelog thread back to client connection thread)
        if (g_ptr->is_exit_event(e_json, player_id)) {
            is_connected = false;
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
    return command.dump();
}

// is_valid_request_header(buf_ptr, len)
//      checks whether a request header is well formatted
//      updates len with the lenght of the request

bool is_valid_request_header(char* buf_ptr, int& len) {
    char equals_sign;
    return sscanf(buf_ptr, client_request_format, &len, &equals_sign) == 2 && equals_sign == '=';
}

// is_empty_buf(ptr)
//      checks if a buffer is empty or has data to read

bool is_empty_buf(char* ptr) {
    return ptr == NULL || ptr[0] == '\0';
}

// read_from_socket(cfd, buf_ptr, sz):
//      reads from socket, blocking
//      returns true if there is text to read from buf_ptr
//      returns false if there is none (i.e. socket connection closed)

bool read_from_socket(int cfd, char* buf_ptr, int sz) {
    // read from socket
    int r = recv(cfd, buf_ptr, sz, 0);
    
    // if error reading from socket, print error and return
    if (r < 0) {
        std::cerr << "Error: error reading from socket" << std::endl;
    } else if (r == 0) {
        std::cout << "Client closed the socket connection" << std::endl;
    }
    
    return r > 0;
}

// write_to_socket(cfd, msg)
//      wrapper around underlying c syscalls
//      returns true if successfully wrote entire message

bool write_to_socket(int cfd, std::string msg) {

    int n_written = write(cfd, msg.c_str(), msg.size());
    if (n_written < msg.size()) {
        std::cerr << "Error writing to client on cdf " << std::to_string(cfd) << std::endl;
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