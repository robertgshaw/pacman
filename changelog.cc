#include "changelog.hh"

void Changelog::push(Event e) {
    queue.push_back(e);
    return;
}   

Event Changelog::pop(int l_id) {
    assert(!is_empty(l_id));               // invariant that queue not empty
    assert(l_id < l_loc.size());        // invariant that player is valid

    int loc = l_loc[l_id];
    l_loc[l_id]++;
    return queue[loc];
}

bool Changelog::is_empty(int l_id) {
    assert(l_id < l_loc.size());        // invariant that player is valid
    return l_loc[l_id] != queue.size(); 
}

void Changelog::add_listener(int l_id) {
    assert(l_id == l_loc.size());         // invariant that next player added is p_id
    l_loc.push_back(queue.size());  
    return; 
}
