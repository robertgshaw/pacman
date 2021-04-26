#ifndef BOARD_H
#define BOARD_H

#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include "nlohmann/json.hpp"

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

using json = nlohmann::json;

struct tuple {
    int n1;
    int n2;
};

class Node {
    public:
        int index;
        int player_id;
        std::vector<int> adj;

        // constructor
        Node(int i): index(i), player_id(-1) { }
        
        // get_json representation of the node
        json get_json() {
            json j;
            j["index"] = index;
            json adj_json(adj);
            j["adj"] = adj_json;

            return j;
        }
};

class Board {
    public:
        // constructor
        Board(int w);

        // update the board state
        void add_player(int player_id);
        void move_player(int player_id, int dir);

        // serialize / outputting data
        void print();
        json get_json();

    private:
        // board state
        int width;
        std::vector<Node> nodes;
        std::vector<int> p_locations;

        // private helpers / utilites
        void init_graph(int w);
        int get_loc(int i, int j);

};

#endif