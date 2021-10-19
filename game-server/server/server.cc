#include <stdio.h>
#include <iostream>
#include <thread>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../shared/nlohmann/json.hpp"

#include "server_api.hh"

using json = nlohmann::json;

// ENTRY POINT
// main(argc, argv)
//      starts up the game server
//      listens to the command line for user input --- currently only 'exit' implemented

int main(int argc, char** argv) {

    // create a pipe used to signal exit
    int pfds[2];
    if (pipe(pfds) != 0) {
        std::cerr << "Error opening pipe for main server thread ... shutting down." << std::endl;
        return 1;
    }

    // spin off thread whcih processes the server
    std::thread server_t  = std::thread(run_server, pfds[0]);

    // on the main thread, get input from user at the server command line
    FILE* command_file = stdin;

    char buf[BUFSIZ];
    int bufpos = 0;
    bool needprompt = true;
    bool exited = false;

    while (!exited && !feof(command_file)) {

        // read a string, checking for error or EOF
        if (fgets(&buf[bufpos], BUFSIZ - bufpos, command_file) == nullptr) {
            if (ferror(command_file) && errno == EINTR) {
                // ignore EINTR errors
                clearerr(command_file);
                buf[bufpos] = 0;
            } else {
                if (ferror(command_file)) {
                    std::cerr << "Error reading the user input inside main";
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
    server_t.join();                    // server thread

    if (close(pfds[0]) < 0) {           // exit pipe read end
        std::cerr << "Error: close failed for pfds[0]" << std::endl;
    }
    if (close(pfds[1]) < 0) {           // exit pipe write end
        std::cerr << "Error: close failed for pfds[1]" << std::endl;
    }
    if (fclose(command_file) < 0) {     // command file
        std::cerr << "Error: close failed for command file" << std::endl;
    }
    
	return 0;
}