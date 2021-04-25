#include "game.hh"

// Game(int w):
//      Constructor. Creates 2D graph of size W x W   

Game::Game(int w) : board_(Board(w)), n_players(0) {
}

// Game::create_player(int cfd, int index)
//      Creates new player and adds to player vector

void Game::create_player(int cfd) {
    n_players++;
    Player new_player = Player(cfd, n_players);
    players_.push_back(new_player);
    
    // invariant that n_players should equal size of player vector
    assert(n_players == players_.size());
}

// Game::get_player(int i)
//      gets the ith player from the players vector
Player* Game::get_player(int i) {
    return &(players_[i]);
}

// std::string Game::get_board_json()
//      Converts board into a json object
//      Returns json 

std::string Game::get_board_json() {
    return board_.get_json();
}