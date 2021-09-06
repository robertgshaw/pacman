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

    fd_set readfds;
    int max_fd = sfd > exit_pipe_fd ? sfd : exit_pipe_fd;

    bool is_exited = false;

    // accept client connection + spin off connection threads until exit
    while(!is_exited) {
        std::cout << "before select" << std::endl;
        FD_ZERO(&readfds);
        FD_SET(exit_pipe_fd, &readfds);    // add exit signal pipe
        FD_SET(sfd, &readfds);             // add the socket

        // accept new connections
        int r = select(max_fd + 1 , &readfds , NULL , NULL , NULL);

        std::cout << "after select" << std::endl;
        
        if (r < 0) {
            std::cerr << "TODO: shutdown the board on error";
        } else if (r == 0) {
            std::cerr << "should not get here";
        } else if (FD_ISSET(exit_pipe_fd, &readfds)) {
            is_exited = true;
            char rbuf[BUFSIZ];
            
            size_t n = read(exit_pipe_fd, rbuf, BUFSIZ);
            assert(n >= 0);
            rbuf[n] = 0;

            printf("Read %s\n", rbuf);

        } else {
            // std::cout << "here2" << std::endl;
            assert(FD_ISSET(sfd, &readfds));
            int cfd = accept(sfd, (sockaddr*) &client, &c);
            if (cfd < 0) {

                // TODO: tell threads that we are dead + that it should shut down
                perror("accept");
                close(cfd);
                close(sfd);
                return;
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
            // should not get here
            std::cerr << "Error: Thread " << connection.get_id() << " could not be joined" << std::endl;
        }
    }

    // cleanup all the exit pipe resources
    exitpipe_.close_all();
    close(exit_pipe_fd);

    // close the sfd    
    shutdown(sfd, SHUT_RDWR);
    if(close(sfd) < 0) {
        std::cerr << "Error: close failed for sfd:" << std::endl;
    }
}

int main(int argc, char** argv) {

    // initialzie socket
    std::cout << "Initializing socket ...";    
    int sfd = init_socket(port, max_players);
    if (sfd == -1) {
        std::cout << "[FAILED] ... Shutting down\n" << std::endl;
        return 1;
    } else {
        std::cout << "[DONE] ... Listening on port " << std::to_string(port) << std::endl;
    }

    // initialize game, spinning off a thread to run the game server
    //      create a pipe to send exit messages from the active process to the run_server thread
    std::cout << "Initializing game ...";
    int pfds[2];
    if (pipe(pfds) != 0) {
        std::cerr << "Error opening pipe for main server thread ... shutting down." << std::endl;
        return 0;
    }

    std::thread server_t  = std::thread(run_server, sfd, pfds[0]);
    std::cout << "[DONE]"<< std::endl;

    // listen for commands from the command line
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
            std::cout << cmd.substr(0,4) << std::endl;
            if (cmd.substr(0,4) == "exit") {

                std::cout << "in exit" << std::endl;
                char wbuf[BUFSIZ];
                sprintf(wbuf, "exit");
                size_t n = write(pfds[1], wbuf, strlen(wbuf));
                std::cout << "wrote n = " << std::to_string(n) << " bytes to pipe" << std::endl;
                exited = true;
                
            }
            bufpos = 0;
            needprompt = true;
        }
    }

    server_t.join();
    close(pfds[1]);

	return 0;
}