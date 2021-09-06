#ifndef EXITPIPE_H
#define EXITPIPE_H

#include <vector>
#include <unistd.h>
#include <cstring>
#include <iostream>

struct pipe_fd_pair_t {
    int reader;
    int writer;
};

class Exitpipe {
    public:
        void add_fd_pair();
        int pop_reader_fd();
        void exit_all();
        void close_all();

    private:
        std::vector<pipe_fd_pair_t> fd_pairs;
};


#endif