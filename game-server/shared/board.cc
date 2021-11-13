#include "board.hh"

using json = nlohmann::json;

//
//
// ******** INIT UNDERLYING GRAPH *********
//
//

// void init_graph(int w):
//      inits 2d graph of size w*w, with all edges to adjacent nodes in the graph connected
//      called on the server side

void Board::init_graph(int w) {
    assert(w > 10);
    assert(w % 2 == 1);
    width = w;
    pacman_id = NO_PACMAN_ID_SET;
    pacman_score = 0;

    nodetype t;
    int y, x;

    // iterate through all the nodes --- setting types and indexes
    for (int i = 0; i < w*w; i++) {
        std::tie(y,x) = get_yx(i);
        
        if (x == 0 || x == w - 1 || y == 0 || y == w - 1) {
            t = blocked;
        } else if ((x == w/4 || x == w/4 + 1 || x == w/2 || x == 3*w/4 || x == 3*w/4 + 1) && (y != 1 && y != w-2)) {
            t = blocked;
        } else {
            t = coin;
        }

        // add to node_arr
        Node nd = Node(i, t);
        nodes.push_back(nd);
    }
    
    // iterate through all the nodes --- setup the edges for the graph
    int loc;
    for (int i = 0; i < w*w; i++) {
        std::tie(y,x) = get_yx(i);
        
        // top edge
        if (y == 0) {
            nodes[i].adj[UP] = EMPTY;
        } else { 
            loc = get_loc(y - 1, x);
            nodes[i].adj[UP] = nodes[loc].node_type == blocked ? EMPTY : loc;
        }

        // right edge
        if (x % w == w - 1) {
            nodes[i].adj[RIGHT] = EMPTY;
        } else {
            loc = get_loc(y, x + 1);
            nodes[i].adj[RIGHT] = nodes[loc].node_type == blocked ? EMPTY : loc;
        }

        // down edge
        if (y == w - 1) {
            nodes[i].adj[DOWN] = EMPTY;
        } else {
            loc = get_loc(y + 1, x);
            nodes[i].adj[DOWN] = nodes[loc].node_type == blocked ? EMPTY : loc;
        }

        // left edge
        if (x % w == 0) {
            nodes[i].adj[LEFT] = EMPTY;
        } else {
            loc = get_loc(y, x - 1);
            nodes[i].adj[LEFT] = nodes[loc].node_type == blocked ? EMPTY : loc;
        }
    }
}

//
//
// ******** UPDATE STATE *********
//
//

void Board::set_pacman(int player_id) {
    assert(p_locations[player_id] != EMPTY);
    assert(pacman_id == NO_PACMAN_ID_SET);

    pacman_id = player_id;

    return;
}

// int add_player(player_id)
//      adds player_id to the game at random i,j by: 
//      adds player_id into the node vector
//      adds player_id into the p_locations vector
//      returns the location where the player was inserted

int Board::add_player(int player_id) {

    // choose random location to start at, redoing until one is open
    int ind;
    do {
        ind = get_loc(std::rand() % width, std::rand() % width);
    } while (nodes[ind].player_id != EMPTY || nodes[ind].node_type == blocked);

    return add_player(player_id, ind);
}

// int add_player(player_id, loc)
//      adds player_id to the game at loc by: 
//      adds player_id into the node vector
//      adds player_id into the p_locations vector
//      returns the location where the player was inserted

int Board::add_player(int player_id, int loc) {
    assert(nodes[loc].player_id == EMPTY);

    // insert into the nodes tracker
    nodes[loc].player_id = player_id;

    // insert into the p_locations tracker
    assert(player_id == p_locations.size());
    p_locations.push_back(loc);

    return loc;
}

// int is_move_kill(player_id, dir) 
//      checks if a move results in the pacman's death
//      returns -1 if not
//      returns the player_id of the pacman if yes

int Board::is_move_kill(int player_id, int dir) {
    // invariant that direction is UP (0), RIGHT (1), DOWN (2), or LEFT (3)
    assert(dir < NDIR);

    // invariant that the player_id passed is an actual player
    assert(player_id < p_locations.size() && p_locations[player_id] != EMPTY);

    // get current loc of the player
    int loc = p_locations[player_id];
    assert(nodes[loc].player_id == player_id); // both representations of data should be consistent

    // get neighbor in the dir direction
    int new_loc = nodes[loc].adj[dir];

    // if move invalid or new_loc is empty, not a collision
    if (new_loc == EMPTY || nodes[new_loc].player_id == EMPTY) {
        return -1;
    // if the pacman is either at loc or new_loc, the move killed the pacman
    } else if (player_id == pacman_id || nodes[new_loc].player_id == pacman_id) {
        return pacman_id;
    // otherwise, the pacman wasn't involved
    } else {
        return -1;
    }
}

// struct locpair move_player(player_id)
//      moves player from current location updating in:
//      nodes vector -- updating nodes.player_id
//      p_locations vector -- updating p_locations[player_id]
//      returns a locpair = {old loc, new loc}

struct locpair Board::move_player(int player_id, int dir) {

    // invariant that direction is UP (0), RIGHT (1), DOWN (2), or LEFT (3)
    assert(dir < NDIR);

    // invariant that the player_id passed is an actual player
    assert(player_id < p_locations.size() && p_locations[player_id] != EMPTY);

    // get current loc of the player
    int loc = p_locations[player_id];
    assert(nodes[loc].player_id == player_id); // both representations of data should be consistent

    // get neighbor in the dir direction
    int new_loc = nodes[loc].adj[dir];

    // if move was valid and we have a new loc, update state
    if (new_loc != EMPTY && nodes[new_loc].player_id == EMPTY) {
        // update nodes vector
        nodes[new_loc].player_id = player_id;
        nodes[loc].player_id = EMPTY;

        // if player that moved was the pacman, pickup the coin
        if (player_id == pacman_id && nodes[new_loc].node_type == coin) {
            nodes[new_loc].node_type = open;
            pacman_score++;
        }

        // update p_locations vector
        p_locations[player_id] = new_loc;

        return {loc, new_loc};  // move successful
    }

    return {loc, loc};          // move not successful
}

// int delete_player(player_id)
//      deletes player from current location updating in:
//      nodes vector -- updating nodes[loc].player_id = -1
//      p_locations vector -- updating p_locations[player_id] = -1
//      returns location of the deleted player if deleted
//      returns -1 is player has already been deleted
//      returns -2 if player id was invalid

int Board::delete_player(int player_id) {

    // if p_id invalid, return -2
    if (player_id >= p_locations.size()) {
        return -2;

    // if the player has already been deleted, return -1
    } else if (p_locations[player_id] == EMPTY) {
        return EMPTY;

    // otherwise, delete + return the deleted location
    } else {
        int loc = p_locations[player_id];
        nodes[loc].player_id = EMPTY;
        p_locations[player_id] = EMPTY;

        if (pacman_id == player_id) {
            pacman_id = NO_PACMAN_ID_SET;
        }

        return loc;
    }
}

//
//
// ******** STATE GETTERS *********
//
//

// bool is_active
//      returns true if a player is active

bool Board::is_active(int player_id) {
    return p_locations[player_id] != EMPTY;
}

// int get_width()
//      returns width of the board

int Board::get_width() {
    return width;
}

// int get_pacman_id()
//      returns pacman_id

int Board::get_pacman_id() {
    return pacman_id;
}

bool Board::is_pacman_id_set() {
    return pacman_id != NO_PACMAN_ID_SET;
}

// int get_node_player
//      returns the player id at node i

int Board::get_node_player(int i) {
    return nodes[i].player_id;
}


// int get_node_type
//      returns the typeof the node i

nodetype Board::get_node_type(int i) {
    return nodes[i].node_type;
}

// int, int get_yx(int loc)
//      returns the yx coordinates of location loc

std::tuple<int, int> Board::get_yx(int loc) {
    int y = loc / width;
    int x = loc % width;
    return std::make_tuple(y,x);
}

//
//
// ******** SERIALIZING THE STATE *********
//
//

// void print():
//      prints content of board to terminal

void Board::print() {
    std::string str = "";
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < width; j++) {
            int loc = get_loc(i,j);
            if (nodes[loc].player_id != EMPTY) {
                str.append(" ");
                str.append(std::to_string(nodes[loc].player_id));
                str.append(" ");
            } else if (nodes[loc].node_type == coin) {
                str += " C ";
            } else if (nodes[loc].node_type == blocked) {
                str += " B ";
            } else {
                str += " x ";
            }
        }
        str += "\n";
    }

    std::cout << str;
}

// std::string get_json() 
//      returns json representation of the board

json Board::get_json() {
    json j;
    j["width"] = width;
    j["pacman_id"] = pacman_id;
    j["pacman_score"] = pacman_score;
    j["n_players"] = p_locations.size();
    j["nodes"] = json::array();
    
    for (Node& node: nodes) {
        j["nodes"].push_back(node.get_json());
    }

    return j;
}

/*
 *
 ***** Private Helper Functions *****
 *
 */

// int get_loc(int i, int j):
//      gets the index of the node at row i, column j

int Board::get_loc(int i, int j) {
    assert(i < width);
    assert(j < width);
    return nodes[i * width + j].index;
}