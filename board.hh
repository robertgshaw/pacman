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

struct locpair {
    int old_loc;
    int new_loc;
};

class Node {
    public:
        int index;
        int player_id;
        std::vector<int> adj;

        // constructor
        Node(int i): index(i), player_id(-1) { }
        
        // get_json representation of the node
        nlohmann::json get_json() {
            nlohmann::json j;
            j["index"] = index;
            j["player_id"] = player_id;
            nlohmann::json adj_json(adj);
            j["adj"] = adj_json;

            return j;
        }

        void from_json(nlohmann::json node_json) {
            player_id = node_json["player_id"];
            adj = node_json["adj"].get<std::vector<int>>();
        }
};

class Board {
    public:
        void init_graph(int w);
        void init_graph(nlohmann::json board_json);

        int get_width();
        int get_node_player(int i);
        bool has_quit(int player_id);
        std::tuple<int,int> get_yx(int loc);

        // update the board state
        int add_player(int player_id);
        int add_player(int player_id, int loc);
        struct locpair move_player(int player_id, int dir);
        int delete_player(int player_id);

        // serialize / outputting data
        void print();
        nlohmann::json get_json();

    private:
        // board state
        int width;
        std::vector<Node> nodes;
        std::vector<int> p_locations;

        // private helpers / utilites
        int get_loc(int i, int j);
};

#endif