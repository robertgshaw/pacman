#include "player.hh"

// Player(int client_fd, int ind)
//      Constructor

Player::Player(int client_fd, int ind) : cfd(client_fd), id(ind) { }

/* 
 *  
 ******* API CALLS *********
 *
 */

// Player.send_board()
//      sends message to client conforming to API, which is a json object {command: args}
//      message sent to socket is then: {board: board_json}
//      returns true if success, false if fail

bool Player::send_board(std::string board_json) {
    return write_to_socket(board_json);
}

/*
 *
 ******* HELPERS *********
 *
 */

// write_to_socket(std::string msg)
//      performs underlying syscalls to implement API
//      blocking call (calls write)
//      returns true if entire msg written to socket

bool Player::write_to_socket(std::string msg) {
    int n_written = write(cfd, msg.c_str(), msg.size());

    if (n_written < msg.size()) {
        fprintf(stderr, "Error writing to client: %d on cdf: %d\n\n", id, cfd);
    }

    return n_written == msg.size();
}


