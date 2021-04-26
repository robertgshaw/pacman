#include "player.hh"

using json = nlohmann::json;

// Player(int client_fd, int ind)
//      Constructor

Player::Player(int client_fd, int ind) : cfd(client_fd), id(ind) { }

// handle_connection()
//      Reads from the socket for any client requests
//      Sends client events happening in the game 

void Player::handle_connection() {

    char buf[BUFSIZ];
    char* buf_ptr = buf;
    int len, nread, body_start_ind, body_len_in_buf;
    bool skip_get = false;
    char equals_sign;
    std::string request;

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

            // IF(buffer contained entire request), handle request + loop back to handle next request
            if (len == body_len_in_buf) {
                handle_request(json::parse(request));
                
                // move the buffer pointer over to the start of the next request, update skip flag
                buf_ptr += body_start_ind + len;
                skip_get = (sscanf(buf_ptr, "REQUEST len=%d, body%c", &len, &equals_sign) == 2 && equals_sign == '=');

            // IF(buffer did not contain the entire request), read until we have the whole thing
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

// handle_request(request) 
//      Handle request from the client

void Player::handle_request(json request) {
    std::cout << request.dump(4) << std::endl;
}