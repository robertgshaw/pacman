#include "board.hh"

using json = nlohmann::json;

/*
 *
 ***** Update Board State *****
 *
 */

// void init_graph(int w):
//      no CRITICAL SECTION (this is called before threads launched)
//      inits 2d graph of size w*w, with all edges to adjacent nodes in the graph connected

void Board::init_graph(int w) {
    width = w;
    for (int i = 0; i < w*w; i++) {     // iterate through all the nodes
        Node nd = Node(i);

        if (i >= w) {
            nd.adj.push_back(i - w);    // add up edge
        }
        
        if (i % w != 0) {
            nd.adj.push_back(i - 1);    // add left edge
        }
        
        if (i % w != w - 1) {
            nd.adj.push_back(i + 1);    // add right edge
        }
        
        if (i < w*(w-1)) {
            nd.adj.push_back(i + w);    // add bottom edge
        }
        
        nodes.push_back(nd);            // add to node_arr
    }
}

// add_player(player_id)
//      has CRITICAL SECTION
//      adds player_id to the game at random i,j by: 
//      adds player_id into the node vector
//      adds player_id into the p_locations vector

void Board::add_player(int player_id) {
    
    // choose random location to start at
    int ind = get_loc(std::rand() % width, std::rand() % width);

    // CRITICAL SECTION ------ shared state across all the players
    std::lock_guard<std::mutex> lock(b_mutex);
    
    // insert into the nodes tracker
    nodes[ind].player_id = player_id;

    // insert into the p_locations tracker
    assert(player_id == p_locations.size());
    p_locations.push_back(ind);

    return;
}

// move_player(player_id)
//      has CRITICAL SECTION
//      moves player from current location updating in:
//      nodes vector -- updating nodes.player_id
//      p_locations vector -- updating p_locations[player_id]

void Board::move_player(int player_id, int dir) {
    std::cout << "move player called by pid " << std::to_string(player_id) << std::endl;

    // invariant that direction is UP (0), RIGHT (1), DOWN (2), or LEFT (3)
    assert(dir < 4);
    
    // invariant that the player_id passed is an actual player
    assert(player_id < p_locations.size());

    // CRITICAL SECTION --- accessing / updating shared state
    std::lock_guard<std::mutex> lock(b_mutex);

    // get current loc of the plaer
    int loc = p_locations[player_id];
    assert(nodes[loc].player_id == player_id); // both representations of data should be consistent

    // set new_loc post move, if move was valid
    int new_loc = loc;
    if (dir == UP && loc >= width) {                            // loc < width => top row
        new_loc = loc - width;
    } else if (dir == RIGHT && (loc % width) != width - 1) {    // loc mod width == width - 1 => right side
        new_loc = loc + 1;
    } else if (dir == LEFT && loc % width != 0) {               // loc mod width == 0 => left side
        new_loc = loc - 1;
    } else if (dir == DOWN && loc < width * (width - 1)) {      // loc > width * width - 1 => bottom row
        new_loc = loc + width;
    }
    
    std::cout << "new loc = " << std::to_string(new_loc) << std::endl;
    std::cout << "player id = " << std::to_string(nodes[new_loc].player_id) << std::endl;

    // if move was valid and we have a new loc, update state
    if (new_loc != loc && nodes[new_loc].player_id == -1) {
        // update nodes vector
        nodes[new_loc].player_id = player_id;
        nodes[loc].player_id = -1;

        // update p_locations vector
        p_locations[player_id] = new_loc;
    }

    return;
}

/*
 *
 ***** Serialize / Printing State of Board *****
 *
 */

// void print():
//      prints content of board to terminal

void Board::print() {
    std::string str = "";
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < width; j++) {
            int loc = get_loc(i,j);
            if (nodes[loc].player_id != -1) {
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

// std::string get_board_json() 
//      returns a string with json representation of the board

json Board::get_json() {
    json j;
    j["width"] = width;
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
    assert(i*j < width * width);
    return nodes[i * width + j].index;
}