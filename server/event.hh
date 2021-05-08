#ifndef EVENT_H
#define EVENT_H

#include "../shared/nlohmann/json.hpp"

class Event {
    public:
        Event(int p, int q) : player_id{p}, modifier{q} {}
        
        // gets the json form of the event
        virtual nlohmann::json get_json() = 0;

        // checks if the json form of the event is a quit for player_id 
        static bool is_quit_event(int player_id, nlohmann::json e_json) {
            if (e_json.find("quit") == e_json.end()) {
                return false;
            } else {
                return e_json["quit"]["pid"] == player_id;
            }
        } 

        int player_id;
        int modifier;   
};

class Move : public Event {
    public:
        using Event::Event;
        nlohmann::json get_json();
};

class Add : public Event {
    public:
        using Event::Event;
        nlohmann::json get_json();
};

class Quit : public Event {
    public:
        using Event::Event;
        nlohmann::json get_json();
};

#endif