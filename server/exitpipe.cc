#include "exitpipe.hh"

void Exitpipe::add_fd_pair() { 
    int pfds[2];
    int r = pipe(pfds);
    if (r != 0) {
        // TODO: handle error
    } 

    // setup pipes for both the request handler + the changelpg handler
    pipe_fd_pair_t fd_pair;
    fd_pair.writer = pfds[1];
    fd_pair.reader = pfds[0];
    fd_pairs.push_back(fd_pair);

    return;
}

int Exitpipe::pop_reader_fd() { 
    return fd_pairs.back().reader;
}

void Exitpipe::exit_all() { 
    for (pipe_fd_pair_t fd_pair : fd_pairs) {
        char wbuf[BUFSIZ];
        sprintf(wbuf, "exit");
        size_t n = write(fd_pair.writer, wbuf, strlen(wbuf));
        std::cout << "wrote n = " << std::to_string(n) << " bytes to pipe" << std::endl;
    }
}

void Exitpipe::close_all() { 
    for (pipe_fd_pair_t fd_pair : fd_pairs) {
        int r = close(fd_pair.reader);
        if (r != 0) {
            std::cerr << "Error: error closing reader end of exit pipe" << std::endl;
        }
        
        int q = close(fd_pair.writer);
        if (q != 0) {
            std::cerr << "Error: error closing writer end of exit pipe" << std::endl;
        }
    }
}