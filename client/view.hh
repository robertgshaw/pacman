#include <iostream>
#include <cstdio>
#include <tuple>
#include <ncurses.h>

#include "../shared/board.hh"

struct point {
    int x;
    int y;
};

// UI version of a node on the board graph
class Cell {
    public:
        Cell(int p_id, int y, int x, int c_h, int c_w, bool a, nodetype t, FILE* f) :
            player_id{p_id}, screen_y{y}, screen_x{x}, cell_h{c_h}, cell_w{c_w}, active{a}, type{t}, log_fd{f} {
            
            w_ptr = newwin(cell_h, cell_w, screen_y, screen_x);
            active ? highlight() : draw();
        } 

        void highlight() {
            wattron(w_ptr, A_STANDOUT);
            draw();
            wattroff(w_ptr, A_STANDOUT);
        }

        void destroy() {
            wborder(w_ptr, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
            wrefresh(w_ptr);
            delwin(w_ptr);
        }

        void draw() {
            if (type == blocked) {
                box(w_ptr, 0, 0);
            }
            struct point xy = get_center();
            mvwprintw(w_ptr, xy.y, xy.x, get_icon());
            wrefresh(w_ptr);
        }

        void update(int p_id, bool a) {
            player_id = p_id;
            active = a;
            active ? highlight() : draw();
        }

    private:
        WINDOW* w_ptr;
        bool active;
        nodetype type;
        int player_id;
        int screen_x;
        int screen_y;
        int cell_w;
        int cell_h;
        FILE* log_fd;
        
        struct point get_center() {
            return {cell_w / 2, cell_h / 2};
        }

        const char* get_icon() {
            std::string str;
            if (player_id == -1) {
                str = type == blocked ? "" : " ";
            } else {
                assert(type == open);
                str = std::to_string(player_id);
            }
            return str.c_str();
        }

};

class View {
    public:
        bool init(Board* b_ptr, int pid);
        char get_user_input();
        void update_cell(int ind, int p_id);
        ~View();
        
    private: 
        FILE* log_fp;

        int board_w;
        int board_h;
        int cell_w;
        int cell_h;
        int c_pid;

        std::vector<Cell> cells;
        void destroy_board();
        std::tuple<int, int> convert_yx(int y, int x);

};