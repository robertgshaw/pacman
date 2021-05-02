#ifndef PLAYER_H
#define PLAYER_H

#include <mutex>
#include "board.hh"
#include "nlohmann/json.hpp"


class Player {
    public:


    private:
        mutex b_mutex;        
        Board board_;
}

#endif