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
    assert( w > 10 );
    assert (w % 2 == 1);
    width = w;

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
            t = open;
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
            nodes[i].adj[UP] = nodes[loc].type == blocked ? EMPTY : loc;
        }

        // right edge
        if (x % w == w - 1) {
            nodes[i].adj[RIGHT] = EMPTY;
        } else {
            loc = get_loc(y, x + 1);
            nodes[i].adj[RIGHT] = nodes[loc].type == blocked ? EMPTY : loc;
        }

        // down edge
        if (y == w - 1) {
            nodes[i].adj[DOWN] = EMPTY;
        } else {
            loc = get_loc(y + 1, x);
            nodes[i].adj[DOWN] = nodes[loc].type == blocked ? EMPTY : loc;
        }

        // left edge
        if (x % w == 0) {
            nodes[i].adj[LEFT] = EMPTY;
        } else {
            loc = get_loc(y, x - 1);
            nodes[i].adj[LEFT] = nodes[loc].type == blocked ? EMPTY : loc;
        }
    }
}

// void init_graph(int w):
//      inits 2d graph of size w*w, with all edges to adjacent nodes in the graph connected
//      inserts players into the graph
//      called on the cleint side

void Board::init_graph(json board_json) {
    
    // TDD: invariant that we are passed right API syntax 
    assert(board_json.find("width") != board_json.end());
    assert(board_json.find("n_players") != board_json.end());
    assert(board_json.find("nodes") != board_json.end());
    
    // extract width + number of players
    width = board_json["width"];
    std::vector<int> vect(board_json["n_players"], EMPTY);
    p_locations = vect;

    // extract the nodes from the json
    for(auto it : board_json["nodes"]) {
        Node nd(it["index"], it["type"]);
        nd.from_json(it);
        if (nd.player_id != EMPTY) {
            p_locations[nd.player_id] = nd.index;
        }

        nodes.push_back(nd);
    }
}

//
//
// ******** UPDATE STATE *********
//
//

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
    } while (nodes[ind].player_id != EMPTY);

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

    // get current loc of the plaer
    int loc = p_locations[player_id];
    assert(nodes[loc].player_id == player_id); // both representations of data should be consistent

    // get neighbor in the dir direction
    int new_loc = nodes[loc].adj[dir];

    // if move was valid and we have a new loc, update state
    if (new_loc != EMPTY && nodes[new_loc].player_id == EMPTY) {
        // update nodes vector
        nodes[new_loc].player_id = player_id;
        nodes[loc].player_id = EMPTY;

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
//      returns location of the deleted player if deleted or -1 if not in the game 

int Board::delete_player(int player_id) {

    // if p_id invalid or player already deleted, return -1
    if (player_id >= p_locations.size() || p_locations[player_id] == EMPTY) {
        return EMPTY;

    // otherwise, delete + return the deleted location
    } else {
        int loc = p_locations[player_id];
        nodes[loc].player_id = EMPTY;
        p_locations[player_id] = EMPTY;
        return loc;
    }
}

//
//
// ******** STATE GETTERS *********
//
//

// bool had_quit
//      returns true if a player had already quit the game

bool Board::has_quit(int player_id) {
    return p_locations[player_id] == EMPTY;
}

// int get_width()
//      returns width of the board

int Board::get_width() {
    return width;
}

// int get_node_player
//      returns the player id at node i

int Board::get_node_player(int i) {
    return nodes[i].player_id;
}

// int get_node_type
//      returns the typeof the node i

nodetype Board::get_node_type(int i) {
    return nodes[i].type;
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
            } else {
                str += " X ";
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