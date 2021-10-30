#include "event.hh"

// Move, Add, Quit, Exit inherit from abstract class Event (virtual)
//      this file defines the virtual method (get_json) for each child class
//      then, use the ptr pattern to implement polymorphism 

nlohmann::json Move::get_json() {
    nlohmann::json j;
    j["move"]["pid"] = player_id;
    j["move"]["dir"] = modifier;

    return j;
}

nlohmann::json Add::get_json() {
    nlohmann::json j;
    j["add"]["pid"] = player_id;
    j["add"]["loc"] = modifier;

    return j;
}

nlohmann::json Quit::get_json() {
    nlohmann::json j;
    j["quit"]["pid"] = player_id;
    j["quit"]["loc"] = modifier;
    return j;
}

nlohmann::json Exit::get_json() {
    nlohmann::json j;
    j["exit"]["pid"] = player_id;
    j["exit"]["loc"] = modifier;
    return j;
}