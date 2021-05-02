#ifndef EVENT_H
#define EVENT_H

#include "nlohmann/json.hpp"

class Event {
    public:
        Event(int p, int q) {
            player_id = p;
            modifier = q;
        }

        virtual nlohmann::json get_json() = 0;

        int player_id;
        int modifier;   
};

class Move : public Event {
    public:
        using Event::Event;

        nlohmann::json get_json() {
            nlohmann::json j;
            j["move"]["pid"] = player_id;
            j["move"]["dir"] = modifier;

            return j;
        }
};

class Add : public Event {
    public:
        using Event::Event;

        nlohmann::json get_json() {
            nlohmann::json j;
            j["add"]["pid"] = player_id;
            j["add"]["loc"] = modifier;

            return j;
        }
};

#endif