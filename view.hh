#include <iostream>
#include <tuple>
#include <ncurses.h>
#include "board.hh"

struct point {
    int x;
    int y;
};

class Cell {
    public:
        Cell(int p_id, int y, int x, int c_h, int c_w) :
            player_id{p_id}, screen_y{y}, screen_x{x}, cell_h{c_h}, cell_w{c_w} {
                w_ptr = newwin(cell_h, cell_w, screen_y, screen_x);
                draw();
            } 

        void highlight() {
            wattron(w_ptr, A_BOLD);
            draw();
            wattroff(w_ptr, A_BOLD);
        }

        void destroy() {
            wborder(w_ptr, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
            wrefresh(w_ptr);
            delwin(w_ptr);
        }

        void draw() {
            box(w_ptr, 0, 0);
            struct point xy = get_center();
            mvwprintw(w_ptr, xy.y, xy.x, get_icon());
            wrefresh(w_ptr);
        }

        void update(int p_id) {
            player_id = p_id;
            draw();
        }

    private:
        WINDOW* w_ptr;
        int player_id;
        int screen_x;
        int screen_y;
        int cell_w;
        int cell_h;

        struct point get_center() {
            return {cell_w / 2, cell_h / 2};
        }

        const char* get_icon() {
            std::string str;
            if (player_id == -1) {
                str = '*';
            } else {
                str = std::to_string(player_id);
            }
            return str.c_str();
        }

};

class View {
    public:
        bool init(Board* b_ptr);
        char get_user_input();
        void update_cell(int ind, int p_id);
        ~View();
        
    private: 
        int board_w;
        int board_h;
        int cell_w;
        int cell_h;

        std::vector<Cell> cells;
        void destroy_board();
        std::tuple<int, int> convert_yx(int y, int x);

};