#ifndef EVENT_H
#define EVENT_H

#include "../shared/nlohmann/json.hpp"

class Event {
    public:
        Event(int p, int q) : player_id{p}, modifier{q} {}
        
        // gets the json form of the event
        virtual nlohmann::json get_json() = 0;

        // checks if the json form of the event is a quit for player_id 
        static bool is_exit_event(nlohmann::json e_json) {
            return e_json.find("quit") != e_json.end() || e_json.find("exit") != e_json.end();
        } 

        int player_id;
        int modifier;   
};

// player moves
class Move : public Event {
    public:
        using Event::Event;
        nlohmann::json get_json();
};

// player added to game
class Add : public Event {
    public:
        using Event::Event;
        nlohmann::json get_json();
};

// player quits the game
class Quit : public Event {
    public:
        using Event::Event;
        nlohmann::json get_json();
};

// game is exited
class Exit : public Event {
    public:
        using Event::Event;
        nlohmann::json get_json();
};

#endif