#ifndef CHANGELOG_H
#define CHANGELOG_H

#include <vector>
#include "event.hh"

class Changelog {
    public:
        // API calls
        void push(Event e);             // push event into the changelog
        Event pop(int l_id);            // pop event from the changelog
        bool is_empty(int l_id);        // checks if there are events in the changelog
        void add_listener(int l_id);    // adds listener which will be listening to the queue

    private:
        std::vector<Event> queue;       // queue which will hold the events
        std::vector<int> l_loc;         // vector holding where each listener is in the queue
};

#endif