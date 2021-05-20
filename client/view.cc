#include "view.hh"

static const int c_w = 3;
static const int c_h = 2;

View::~View() {
    std::cout << "Shutting down" << std::endl;
    endwin();
}

bool View::init(Board* b_ptr, int pid) {
    log_fp = fopen("log.txt", "w");

    // setup constants that will define the UI
    board_w = b_ptr->get_width();
    board_h = b_ptr->get_width();
    cell_w = c_w;
    cell_h = c_h;
    c_pid = pid;

    // init NCURSES user interface
    initscr();
    noecho();
    curs_set(0);
    cbreak();

    // check screen is large enough to fit the game
    if ((LINES < c_h * board_h + 3) || (COLS < cell_w * board_w)) {
        endwin();
        std::cerr << "Your terminal window is too small for the game" << std::endl;
        return false;
    }

    clear();
    mvprintw(cell_h * board_h + 1, 0, "q=quit; use awsd to move...");
    mvprintw(cell_h * board_h + 2, (cell_w * board_w - 6) / 2, "Pacman");
    refresh();

    // init each cell  
    for (int i = 0; i < board_w * board_h; i++) {
        int player_id = b_ptr->get_node_player(i);

        int y, x;
        std::tie(y,x) = b_ptr->get_yx(i);
        int start_y, start_x;
        std::tie(start_y, start_x) = convert_yx(y, x);
        
        // create cell, printing to the screen
        Cell cell_(player_id, start_y, start_x, cell_h, cell_w, player_id == c_pid, b_ptr->get_node_type(i), log_fp);
        cells.push_back(cell_);

    }
    fprintf(log_fp, "done print board! \n\n\n\n\n");
    return true;
}

char View::get_user_input() {
    return getch();
}

void View::destroy_board() {
    for (auto cell : cells) {
        cell.destroy();
    }
}

void View::update_cell(int ind, int p_id) {
    fprintf(log_fp, "Updating ind = %d with p_id = %d\n", ind, p_id);
    cells[ind].update(p_id, p_id == c_pid);
}

std::tuple<int, int> View::convert_yx(int y, int x) {
    return std::make_tuple(y * cell_h, x * cell_w);
}