#include "changelog.hh"

void Changelog::push(std::unique_ptr<Event> e_ptr) {
    queue.push_back(std::move(e_ptr));
    return;
}   

nlohmann::json Changelog::pop(int l_id) {
    assert(!is_empty(l_id));
    assert(l_id < l_loc.size());

    int loc = l_loc[l_id];
    l_loc[l_id]++;

    return queue[loc]->get_json();
}

bool Changelog::is_empty(int l_id) {
    assert(l_id < l_loc.size());
    assert(l_loc[l_id] <= queue.size());

    return l_loc[l_id] == queue.size();  
}

void Changelog::add_listener(int l_id) {
    assert(l_id == l_loc.size());
    l_loc.push_back(queue.size());  
    return; 
}
