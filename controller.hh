#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <string>
#include "nlohmann/json.hpp"

#include "board.hh"
#include "view.hh"

#define UPARROW 'w'
#define DOWNARROW 's'
#define LEFTARROW 'a'
#define RIGHTARROW 'd'

class Controller {
    public:
        Controller(nlohmann::json board_json);
        
        // user event handler
        char get_next_move();

        // changelog event handlers
        void handle_event_move(int pid, int dir);
        void handle_event_add(int pid, int loc);
        void handle_event_quit(int pid, int loc);

        bool should_quit();
        void set_quit();

    private:
        // model + view classes
        Board board_;
        View view_;
    
        bool quit;
};

#endif