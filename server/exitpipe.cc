#include "exitpipe.hh"

void Exitpipe::add_fd_pair() { 
    int pfds[2];
    int r = pipe(pfds);
    if (r != 0) {
        // TODO: handle error
    } 

    // setup pipes for both the request handler + the changelpg handler
    pipe_fd_pair_t fd_pair;
    fd_pair.writer = pfds[0];
    fd_pair.reader = pfds[1];
    fd_pairs.push_back(fd_pair);

    return;
}

int Exitpipe::pop_reader_fd() { 
    return fd_pairs.back().reader;
}

void Exitpipe::cleanup_writers() { 
    for (pipe_fd_pair_t fd_pair : fd_pairs) {
        close(fd_pair.writer);
    }
}