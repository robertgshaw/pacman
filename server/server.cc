#include <stdio.h>
#include <iostream>
#include <thread>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../shared/nlohmann/json.hpp"

#include "server_api.hh"
//#include "game.hh"

static const int port = 6169;
static const int board_size = 21;
static const int max_players = 5;

using json = nlohmann::json;

int main(int argc, char** argv) {

    // create socket, bind, and listen 
    int sfd = init_socket(port, max_players);
    if (sfd == -1) {
        std::cout << "Could not init socket...shutting down\n";
        return 1;
    } else {
        std::cout << "Listening on port " << std::to_string(port) << "..." << std::endl;
    }

    // create game obj, which will hold the shared state
    Game game_(board_size);
    
    // create exitpipe obj, which will be use to tell worker threads to exit, if ended
    Exitpipe exitpipe_;

    // create vector of connection threads to handle each user
    std::vector<std::thread> connections;

    // vars used for the socket
    socklen_t c = sizeof(struct sockaddr);
    struct sockaddr_in client;
    int player_id;
    json board_json;

    // accept client connection + spin off connection threads until exit
    bool is_exited = false;
    while(!is_exited) {

        // accept new connections
        int cfd = accept(sfd, (sockaddr*) &client, &c);
        if (cfd < 0) {

            // TODO: tell threads that we are dead + that it should shut down
            perror("accept");
            close(cfd);
            close(sfd);
            return 1;
        }

        // add a new pipe pair
        exitpipe_.add_fd_pair();
        
        // create player + launch connection threads
        std::tie(player_id, board_json) = game_.handle_add_player(cfd);
        connections.push_back(std::thread(handle_connection, cfd, exitpipe_.pop_reader_fd(), player_id, &game_, board_json));
    } 

    // close the exit notification pipes
    exitpipe_.cleanup_writers();

    // join connection threads back together
    for (std::thread & connection : connections) {
        if (connection.joinable()) {
            std::cout << "Joining thread " << connection.get_id() << std::endl;
            connection.join();
        } else {
            // should not get here
            std::cerr << "Error: Thread " << connection.get_id() << " could not be joined" << std::endl;
        }
    }

    // close the sfd    
    shutdown(sfd, SHUT_RDWR);
    if(close(sfd) < 0) {
        std::cerr << "Error: close failed for sfd:" << std::endl;
    }
	return 0;
}