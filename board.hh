#ifndef BOARD_H
#define BOARD_H

#include <iostream>
#include <string>
#include <vector>

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
        std::string get_json() {
            std::string json("{\"index\":");
            json.append(std::to_string(index));

            json.append(", \"adj\":[");
            for (int i = 0; i < adj.size(); i++) {
                json.append(std::to_string(adj[i]));
                
                if (i < adj.size() - 1) {
                    json.append(", ");
                }
            }

            json.append("]}");
            return json;
        }
};

class Board {
    public:
        Board(int w);
        void print_board(int fd);
        std::string get_json();

    private:
        int width;
        std::vector<Node> nodes;

        void init_graph(int w);
        int get_node(int i, int j);

};

#endif