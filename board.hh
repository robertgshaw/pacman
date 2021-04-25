#ifndef BOARD_H
#define BOARD_H

#include <iostream>
#include <string>
#include <vector>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

struct tuple {
    int n1;
    int n2;
};

class Node {
    public:
        int index;
        std::vector<int> adj;

        // constructor
        Node(int i): index(i) { }
        
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
        Board(int w);
        void print_board(int fd);
        json get_json();

    private:
        int width;
        std::vector<Node> nodes;

        void init_graph(int w);
        int get_node(int i, int j);

};

#endif