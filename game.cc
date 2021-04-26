#include "game.hh"

using json = nlohmann::json;

// Game(int w):
//      Constructor. Creates 2D graph of size W x W   

Game::Game(int w) : board_(Board(w)), n_players(0) {
}

// Player* create_player(int cfd, int index)
//      Creates new player and adds to player vector

Player* Game::create_player(int cfd) {
    Player new_player = Player(cfd, n_players, this);
    players_.push_back(new_player);
    board_.add_player(n_players);

    n_players++;
    
    // invariant that n_players should equal size of player vector
    assert(n_players == players_.size());

    return &players_.back();
}

// void move_player(int i)
//      moves the player in the share board state

void Game::move_player(int player_id, int dir) {
    board_.move_player(player_id, dir);
}

// Player* get_player(int i)
//      gets the ith player from the players vector

Player* Game::get_player(int i) {
    return &(players_[i]);
}

/*
 *
 ****** Print / Serialize State ******
 *
 */

// std::string Game::get_board_json()
//      Converts board into a json object
//      Returns json 

json Game::get_board_json() {
    json j = board_.get_json();
    return j;
}

// void Game::get_board_json()
//      Prints board object out

void Game::print_board() {
    board_.print();
}