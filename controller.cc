#include "controller.hh"

using json = nlohmann::json;

Controller::Controller(json board_json) {

    // TODO: add error checking on the json	

    // init model
	board_.init_graph(board_json); 
    
    // init view
    view_.init(&board_);
}

char Controller::get_next_move() {
    char key = view_.get_user_input();
    switch (key) {
        case UPARROW:
            return UP;
        case DOWNARROW:
            return DOWN;
        case LEFTARROW:
            return LEFT;
        case RIGHTARROW:
            return RIGHT;
        case 'q':
            return 'q';
        default:
            return -1;
    }
}

// EVENT HANDLERS
//      called from changelog listener to handle events from the server
//      updates the model (board_) and the view (view_) for each event
//      Handled Events Are:
//          (a) MOVE: moves a player in a direction
//          (b) ADD: adds a new player into a loc

void Controller::handle_event_move(int pid, int dir) {
    // update model
    struct locpair lp = board_.move_player(pid, dir);
    assert(lp.old_loc != lp.new_loc);

    // update view
    view_.update_cell(lp.old_loc, board_.get_node_player(lp.old_loc));
    view_.update_cell(lp.new_loc, board_.get_node_player(lp.new_loc));
}

void Controller::handle_event_add(int pid, int loc) {
    // update model
    int n_loc = board_.add_player(pid, loc);
    assert(n_loc == loc);
    assert(pid == board_.get_node_player(n_loc));

    // update view
    view_.update_cell(n_loc, pid);
}