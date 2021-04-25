#include "helpers.hh"

using json = nlohmann::json;

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

// std::string format_msg(std::string msg)
//      adds useful headers to the msg
//      message length, others potentially to be added

std::string format_msg(json body) {
    std::string body_str = body.dump();
    std::string msg = "REQUEST len=";
    msg.append(std::to_string(body_str.size()));
    msg.append(", body=");
    msg.append(body_str);

    return msg;
}
