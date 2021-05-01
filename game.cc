#include "game.hh"

using json = nlohmann::json;

// Server Kernel - holds the data structures and implements synchronization
//
//      Data Structures (private):
//          board_: holds the actual graph represening the gameboard
//          changelog_: hold a queue of the events occuring in the game
//
//      Synchronization Objects (priavte):
//          b_mutex: mutex used to lock the board
//          cl_mutex: mutex used to lock the changelog_
//          c_cv: condition variable used to notify threads once queue is non-empty
//
//      Data Structure Manipulation (public): update board state / changelog
//          create_player: adds new player to the board --- creates event
//          move_player: moves the player in the board --- create event
//          get_next_event: blocks until new event in the CL queue
//

// Game::init(int w):
//      Starts the game   
//      Copy Constructor deleted given mutex + game should not be copied

Game::Game(int w) { 
    board_.init_graph(w);
    n_players = 0;

    return;
}

//
//
// ********** Data Structure Manipulation ***********
//
//

// Event get_next_event(int p_id):
//      Uses condition variable to wait for changelog to have new events in the queue
//      Once there is a new event in the queue, pop it and pass to the handler

Event Game::get_next_event(int p_id) {
    std::unique_lock<std::mutex> lock(cl_mutex);
    if (changelog_.is_empty(p_id)) {
        cl_cv.wait(lock);
    }

    return changelog_.pop(p_id);
}

// int create_player(int cfd)
//      CRITCAL SECTION
//      creates new player and adds to player vector
//      pushes the player add to the changelog
//      adds the player as listener to the changelog
//      returns player_id, board_json

std::tuple<int, json> Game::create_player(int cfd) {
    // updating both cl / board, need to lock both avoiding deadlock
    std::lock(b_mutex, cl_mutex);
    std::unique_lock<std::mutex> lock_b(b_mutex, std::adopt_lock);
    std::unique_lock<std::mutex> lock_cl(cl_mutex, std::adopt_lock);

    // add player to the board + push to the changelog
    int loc = board_.add_player(n_players);
    changelog_.push(Add(n_players,loc));

    // add the new player as a lister to the changelog
    changelog_.add_listener(n_players);

    // increment player ctr
    n_players++;

    // return the p_id and current board state
    std::tuple<int, json> ret_tuple = std::make_tuple(n_players - 1, board_.get_json());

    // unlock + notify the other threads of an event
    lock_cl.unlock();
    cl_cv.notify_all();
    
    return ret_tuple;
}

// void move_player(int player_id, int dir)
//      CRITCAL SECTION
//      moves the player in the share board state
//      pushes to the change log + notifys the cv

void Game::move_player(int player_id, int dir) {
    // updating both cl / board, need to lock both avoiding deadlock
    std::lock(b_mutex, cl_mutex);
    std::unique_lock<std::mutex> lock_b(b_mutex, std::adopt_lock);
    std::unique_lock<std::mutex> lock_cl(cl_mutex, std::adopt_lock);

    // update state of the game
    if(board_.move_player(player_id, dir)) {
        
        // if move happened, update changelog + notify other threads 
        changelog_.push(Move(player_id, dir)); 
        lock_cl.unlock();
        cl_cv.notify_all();
    }

    return;
}

//
//
// ********** OUTPUT FUNCTIONS ***********
//
//

json Game::get_board_json() {
    json j = board_.get_json();
    return j;
}

// void Game::get_board_json()
//      Prints board object out

void Game::print_board() {
    board_.print();
}