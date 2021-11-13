#ifndef BOARD_H
#define BOARD_H

#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include "nlohmann/json.hpp"

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3
#define NDIR 4
#define EMPTY -1
#define NO_PACMAN_ID_SET -1

struct locpair {
    int old_loc;
    int new_loc;
};

enum nodetype { coin, open, blocked };

class Node {
    public:
        nodetype node_type;
        int index;
        int player_id;
        std::vector<int> adj;

        // constructor
        Node(int i, nodetype t): index(i), player_id(EMPTY), node_type(t), adj(NDIR, EMPTY) { }
        
        // get_json representation of the node
        nlohmann::json get_json() {
            nlohmann::json j;
            j["index"] = index;
            j["player_id"] = player_id;
            nlohmann::json adj_json(adj);
            j["adj"] = adj_json;
            j["node_type"] = node_type;

            return j;
        }

        void from_json(nlohmann::json node_json) {
            player_id = node_json["player_id"];
            adj = node_json["adj"].get<std::vector<int>>();
            node_type = node_json["node_type"];
        }
};

class Board {
    public:
        // INITIALZES THE UNDERLYING GRAPH
        void init_graph(int w);
        void init_graph(nlohmann::json board_json);

        // UPDATE THE BOARD STATE
        void set_pacman(int player_id);
        int add_player(int player_id);
        struct locpair move_player(int player_id, int dir);
        int delete_player(int player_id);
        int is_move_kill(int player_id, int dir); 

        // STATE GETTERS
        int get_width();
        int get_pacman_id();
        bool is_pacman_id_set();
        int get_node_player(int i);
        nodetype get_node_type(int i);
        bool is_active(int player_id);
        std::tuple<int,int> get_yx(int loc);

        // SERIALIZE AND OUTPUT DATA
        void print();
        nlohmann::json get_json();

    private:
        // BOARD STATE
        int width;
        int pacman_id;
        std::vector<Node> nodes;
        std::vector<int> p_locations;

        // UPDATE THE BOARD STATE HELPER
        int add_player(int player_id, int loc);

        // CONVERT FROM X,Y to index
        int get_loc(int i, int j); // get loc of row i column j
};

#endif