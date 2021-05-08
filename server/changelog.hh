#ifndef CHANGELOG_H
#define CHANGELOG_H

#include <iostream>
#include <vector>
#include <memory>
#include "../shared/nlohmann/json.hpp"
#include "event.hh"

class Changelog {
    public:
        // API calls
        void push(std::unique_ptr<Event> e_ptr);    // push event into the changelog
        nlohmann::json pop(int l_id);               // pop event from the changelog - returns json form 
        bool is_empty(int l_id);                    // checks if there are events in the changelog
        void add_listener(int l_id);                // adds listener which will be listening to the queue

    private:
        std::vector<std::unique_ptr<Event>> queue;  // queue which will hold the events
        std::vector<int> l_loc;                     // vector holding where each listener is in the queue
};

#endif