#include "player.hh"

using json = nlohmann::json;

// Player(int client_fd, int ind)
//      Constructor

Player::Player(int client_fd, int ind) : cfd(client_fd), id(ind) { }

// // handle_connection()
// //      reads from the socket for any client requests
// //      sends client events happening in the game 
// void Player::handle_connection() {
//     FILE* fin = fdopen(cfd, "r");
//     FILE* f = fdopen(cfd, "w");

//     char buf[BUFSIZ;
//     size_t sz;

//     // read from the buffer until we have found a 
//     while (fgets(buf, BUFSIZ, fin)) {
        
//     }

//     fclose(fin); // also closes `f`'s underlying fd
//     fclose(f);
// }


/* 
 *  
 ******* API CALLS *********
 *
 */

// Player.send_board()
//      sends message to client conforming to API, which is a json object {command: args}
//      message sent to socket is then: {board: board_json}
//      returns true if success, false if fail

bool Player::send_board(json board_json) {
    return write_to_socket(format_msg(board_json));
}

/*
 *
 ******* HELPERS *********
 *
 */

// bool write_to_socket(std::string msg)
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

