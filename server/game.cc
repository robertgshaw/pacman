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
//          handle_request_move: moves the player in the board --- create event
//          handle_request_quit: deletes the player in the board --- creates event
//          handle_request_exit: ends the game for all players --- creates event
//          get_next_event: blocks until new event in the CL queue
//

Game::Game(int w) { 
    exit_condition = false;
    board_.init_graph(w);
    n_players = 0;

    return;
}

int Game::get_board_size() {
    int w = board_.get_width(); 
    return w * w;
}

//
//
// ********** Data Structure Manipulation ***********
//
//

// json get_next_event(int p_id):
//      Uses condition variable to wait for changelog to have new events in the queue
//      Once there is a new event in the queue, pop it and pass to the handler

json Game::get_next_event(int player_id) {
    std::unique_lock<std::mutex> lock(cl_mutex);
    while (changelog_.is_empty(player_id)) {
        cl_cv.wait(lock);
    }

    return changelog_.pop(player_id);
}

// bool is_exit_event(json e_json):
//      checks if the event should result in an exit (e.g. quit / exit)

bool Game::is_exit_event(json e_json, int player_id) {
    return Event::is_exit_event(e_json, player_id);
}

// int handle_add_player (int cfd)
//      CRITCAL SECTION
//      creates new player and adds to player vector
//      pushes the player add to the changelog
//      adds the player as listener to the changelog
//      returns player_id, board_json

std::tuple<int, json> Game::handle_add_player(int cfd) {
    // updating / checking cl, board, need to lock all avoiding deadlock
    std::lock(b_mutex, cl_mutex);
    std::unique_lock<std::mutex> lock_b(b_mutex, std::adopt_lock);
    std::unique_lock<std::mutex> lock_cl(cl_mutex, std::adopt_lock);

    // add player to the board + push to the changelog
    int loc = board_.add_player(n_players);    
    changelog_.push(std::make_unique<Add>(Add(n_players,loc)));

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

// void handle_request_move (int player_id, int dir)
//      CRITCAL SECTION
//      moves the player in the share board state
//      pushes to the change log + notifys the cv

void Game::handle_request_move(int player_id, int dir) {
    // updating / checking cl, board, need to lock all avoiding deadlock
    std::lock(b_mutex, cl_mutex);
    std::unique_lock<std::mutex> lock_b(b_mutex, std::adopt_lock);
    std::unique_lock<std::mutex> lock_cl(cl_mutex, std::adopt_lock);

    // try move; if successful, log change in the changelog and alert threads
    struct locpair lp = board_.move_player(player_id, dir);
    if(lp.new_loc != lp.old_loc) { 
        changelog_.push(std::make_unique<Move>(Move(player_id, dir))); 
        lock_cl.unlock(); 
        
        cl_cv.notify_all();
    }

    return;
}

// void handle_request_quit (int player_i)
//      CRITICAL SECTION
//      deletes the player from the share board state
//      pushes to the changelog + notify the cv

void Game::handle_request_quit(int player_id) {
    // updating / checking cl, board, need to lock all avoiding deadlock
    std::lock(b_mutex, cl_mutex);
    std::unique_lock<std::mutex> lock_b(b_mutex, std::adopt_lock);
    std::unique_lock<std::mutex> lock_cl(cl_mutex, std::adopt_lock);
   
    // try delete; if successful, log the chnage in the changelog and alter threads
    int loc = board_.delete_player(player_id);
    if(loc != -1) {
        changelog_.push(std::make_unique<Quit>(Quit(player_id, loc)));
        std::cout << "pushing quit to the changelog ... " << std::endl;
        lock_cl.unlock();
        cl_cv.notify_all();
    }

    return;
}

// void handle_request_exit ()
//      CRITICAL SECTION
//      pushes an exit event from the changelog + notify the cv

void Game::handle_request_exit(int player_id) {
    std::unique_lock<std::mutex> lock(cl_mutex);
    changelog_.push(std::make_unique<Exit>(Exit(player_id, 1)));
    lock.unlock();
    cl_cv.notify_all();

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

void Game::print_board() {
    board_.print();
}