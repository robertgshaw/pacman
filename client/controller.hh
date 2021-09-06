#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <cstdio>
#include <string>
#include "../shared/nlohmann/json.hpp"

#include "../shared/board.hh"
#include "view.hh"

#define UPARROW 'w'
#define DOWNARROW 's'
#define LEFTARROW 'a'
#define RIGHTARROW 'd'

class Controller {
    public:
        Controller(nlohmann::json board_json, int pid);
        
        // user event handler
        char get_next_move();

        // changelog event handlers
        void handle_event_move(int pid, int dir);
        void handle_event_add(int pid, int loc);
        void handle_event_quit(int pid, int loc);

        bool should_quit();
        void set_quit();
        int player_id; 

    private:
        FILE* log_fp;

        // model + view classes
        Board board_;
        View view_;

        // id of the player on this client
        // int player_id; 
    
        bool quit;
};

#endif