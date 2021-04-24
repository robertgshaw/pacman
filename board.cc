#include <cassert>
#include "board.hh"

// Board(int w):
//      Constructor. Creates 2D graph of size W x W     

Board::Board(int w) {
    width = w;
    init_graph(w);
}

// void print_graph(int fd):
//      prints content of graph out to fd i

void Board::print_board(int fd) {
    std::string str = "";
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < width; j++) {
            str += " X ";
        }

        str += "\n";
    }

    std::cout << str;
}

// int get_node(int i, int j):
//      gets the index of the node at row i, column j

int Board::get_node(int i, int j) {
    assert(i*j < width * width);
    return nodes[i * width + j].index;
}

// void init_graph(int w):
//      inits 2d graph of size w*w, with all edges to adjacent nodes in the graph connected

void Board::init_graph(int w) {
    // iterate through all the nodes
    for (int i = 0; i < w*w; i++) {
        struct node nd;
        nd.index = i;

        // add up edge
        if (i >= w) {
            nd.adj.push_back(i - w);
        }

        // add left edge
        if (i % w != 0) {
            nd.adj.push_back(i - 1);
        }

        // add right edge
        if (i % w != w - 1) {
            nd.adj.push_back(i + 1);
        }
        
        // add bottom edge
        if (i < w*(w-1)) {
            nd.adj.push_back(i + w);
        }
        
        // add to node_arr
        nodes.push_back(nd);
    }

    
}