#ifndef BOARD_H
#define BOARD_H

#include <iostream>
#include <string>
#include <vector>

struct tuple {
    int n1;
    int n2;
};

struct node {
    int index;
    std::vector<int> adj;
};

class Board {
    public:
        Board(int w);
        void print_board(int fd);

    private:
        int width;
        std::vector<struct node> nodes;

        void init_graph(int w);
        int get_node(int i, int j);

};

#endif