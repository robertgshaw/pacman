#include "game.hh"

using json = nlohmann::json;

// Game::init(int w):
//      Starts the game   
//      Copy Constructor deleted given mutex + game should not be copied

Game::Game(int w) { 
    board_.init_graph(w);
    n_players = 0;

    return;
}

// void create_player(int cfd, int index)
//      Creates new player and adds to player vector

int Game::create_player(int cfd) {
    // Player new_player = Player(cfd, n_players, this);

    // std::unique_lock<std::mutex> lock(g_mutex);
    // players_.push_back(new_player);
    // lock.unlock();

    board_.add_player(n_players);
    n_players++;

    return n_players - 1;
}

// void move_player(int i)
//      moves the player in the share board state

void Game::move_player(int player_id, int dir) {
    board_.move_player(player_id, dir);
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