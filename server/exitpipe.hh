#ifndef EXITPIPE_H
#define EXIT_PIPE_H

#include <vector>
#include <unistd.h>

struct pipe_fd_pair_t {
    int reader;
    int writer;
};

class Exitpipe {
    public:
        void add_fd_pair();
        int pop_reader_fd();
        void cleanup_writers();

    private:
        std::vector<pipe_fd_pair_t> fd_pairs;
};


#endif