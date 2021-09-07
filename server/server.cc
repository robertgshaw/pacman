#include <stdio.h>
#include <iostream>
#include <thread>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../shared/nlohmann/json.hpp"

#include "server_api.hh"

static const int port = 6169;
static const int board_size = 21;
static const int max_players = 5;

using json = nlohmann::json;

void run_server(int sfd, int exit_pipe_fd) {
    
    // init game objects + connection objects
    Game game_(board_size);
    Exitpipe exitpipe_;                     
    std::vector<std::thread> connections;

    // vars for the main loop below
    struct sockaddr_in client;
    socklen_t c = sizeof(struct sockaddr);
    int player_id;
    json board_json;

    // vars for slect
    fd_set readfds;
    int max_fd = sfd > exit_pipe_fd ? sfd : exit_pipe_fd;

    // break condition
    bool is_exited = false;

    // accept client connection + spin off connection threads until exit
    while(!is_exited) {

        // setup readfds set for select
        FD_ZERO(&readfds);
        FD_SET(exit_pipe_fd, &readfds);    // add exit signal pipe
        FD_SET(sfd, &readfds);             // add the socket

        // accept new connections
        int r = select(max_fd + 1, &readfds, NULL, NULL, NULL);                
        if (r < 0) {
            std::cerr << "TODO: shutdown the board on error" << std::endl;
        } else if (r == 0) {
            std::cerr << "TODO: shutdown the board on timeout:" << std::endl;
        } else if (FD_ISSET(exit_pipe_fd, &readfds)) {
            is_exited = true;
        } else {
            assert(FD_ISSET(sfd, &readfds));
            int cfd = accept(sfd, (sockaddr*) &client, &c);
            if (cfd < 0) {

                // TODO: tell threads that we are dead + that it should shut down
                std::cerr << "Error: accept on sfd returned -1" << std::endl;
                close(cfd);
                is_exited = true;
            }

            // add a new pipe pair
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
            std::cout << "Joining thread " << connection.get_id() << "...";
            connection.join();
            std::cout << "[DONE]" << std::endl;
        } else {
            // should not get here, since we never detach or join otherwise
            // THIS should be the only place we join
            std::cerr << "Error: Thread " << connection.get_id() << " could not be joined" << std::endl;
            assert(0 == 1);
        }
    }

    // cleanup all the exit pipe resources
    exitpipe_.close_all();
}

int main(int argc, char** argv) {

    // initialize socket for exit
    std::cout << "Initializing socket ...";    
    int sfd = init_socket(port, max_players);
    if (sfd == -1) {
        std::cout << "[FAILED] ... Shutting down\n" << std::endl;
        return 1;
    } else {
        std::cout << "[DONE] ... Listening on port " << std::to_string(port) << std::endl;
    }

    // initialize the game
    std::cout << "Initializing game ...";

    // create a pipe used to signal exit
    int pfds[2];
    if (pipe(pfds) != 0) {
        std::cerr << "Error opening pipe for main server thread ... shutting down." << std::endl;
        return 1;
    }

    // spin off thread whcih processes the server
    std::thread server_t  = std::thread(run_server, sfd, pfds[0]);
    std::cout << "[DONE]"<< std::endl;

    // on the main thread, get input from user at the server command line
    FILE* command_file = stdin;

    char buf[BUFSIZ];
    int bufpos = 0;
    bool needprompt = true;
    bool exited = false;

    while (!exited && !feof(command_file)) {
        
        // print the prompt at the beginning of the line
        if (needprompt) {
            std::cout << "pacman$ ";
            fflush(stdout);
            needprompt = false;
        }

        // read a string, checking for error or EOF
        if (fgets(&buf[bufpos], BUFSIZ - bufpos, command_file) == nullptr) {
            if (ferror(command_file) && errno == EINTR) {
                // ignore EINTR errors
                clearerr(command_file);
                buf[bufpos] = 0;
            } else {
                if (ferror(command_file)) {
                    perror("sh61");
                }
                break;
            }
        }

        // if a complete command line has been provided, run it
        bufpos = strlen(buf);
        if (bufpos == BUFSIZ - 1 || (bufpos > 0 && buf[bufpos - 1] == '\n')) {
            std::string cmd(buf); 
            if (cmd.substr(0,4) == "exit") {
                char wbuf[BUFSIZ];
                sprintf(wbuf, "exit");
                size_t n = write(pfds[1], wbuf, strlen(wbuf));
                exited = true;                
            }
            bufpos = 0;
            needprompt = true;
        }
    }

    // free OS resources created by main

    // join the server_t thread
    server_t.join();

    // close the exit pipe
    if (close(pfds[0]) < 0) {
        std::cerr << "Error: close failed for pfds[0]" << std::endl;
    }
    if (close(pfds[1]) < 0) {
        std::cerr << "Error: close failed for pfds[1]" << std::endl;
    }

    // close the command file
    if (fclose(command_file) < 0) {
        std::cerr << "Error: close failed for command file" << std::endl;
    }

    // close the sfd    
    shutdown(sfd, SHUT_RDWR);
    if(close(sfd) < 0) {
        std::cerr << "Error: close failed for sfd" << std::endl;
    }

	return 0;
}