// directions for the game
const UP = 0;
const RIGHT = 1;
const DOWN = 2;
const LEFT = 3;

// holds the board state
class BoardState {
    constructor (width) {
        this.width = width;
        this.squares = Array(this.width * this.width).fill(null);
    }

    // adds player [id] to location [loc]
    addPlayer(id, loc) {
        // confirm add complies with the protocol, logging errors if not
        if (!this.locIsInBoard(loc)) {
            console.log("ERROR: cannot add player to this location since it is not in the board");
        } else if (!this.locIsEmpty(loc)) {
            console.log("ERROR: cannot add player to this location since it is not empty");
        } 

        // add player to the location
        else {
            this.squares[loc] = id;
        } 
    }

    // moves player [id] from location [loc] in direction [UP, DOWN, LEFT, RIGHT]
    movePlayer(id, loc, dir) {       
        switch(dir) {
            case UP:
                this.movePlayerHelper(id, loc, loc - this.width, this.locIsTopRow);
                break;
            case RIGHT:
                this.movePlayerHelper(id, loc, loc + 1, this.locIsRightCol);
                break;
            case LEFT:
                this.movePlayerHelper(id, loc, loc - 1, this.locIsLeftCol);
                break;
            case DOWN:
                this.movePlayerHelper(id, loc, loc + this.width, this.locIsBottomRow);
                break;
            default:
                console.log("ERROR: direction is not UP, DOWN, LEFT, or RIGHT");
        }
    }

    movePlayerHelper(id, loc, newLoc, checkLocIsSide) {
        // confirm move complies with the protocol, logging errors if not
        if (!this.playerIsAtLoc(id, loc)) {
            console.log("ERROR: cannot move player since not at starting location");
        }
        else if (checkLocIsSide(loc)) {
            console.log("ERROR: cannot move player since new location is off the board");
        } 
        else if (!this.locIsEmpty(newLoc)) {
            console.log("ERROR: cannot move player new location is not empty");
        }

        // move player to the new location
        else {
            this.squares[newLoc] = this.squares[loc];
            this.squares[loc] = null;
        }
    }    

    playerIsAtLoc(id, loc) {
        return this.squares[loc] === id;
    }

    locIsEmpty(loc) {
        return this.squares[loc] === null;
    }

    locIsInBoard(loc) {
        return (loc >= 0 && loc < this.width * this.width);
    }

    locIsTopRow(loc) {
        return loc < this.width;
    }

    locIsBottomRow(loc) {
        return (loc >= this.width * (this.width - 1) && loc < this.width * this.width);
    }

    locIsRightCol(loc) {
        return loc % this.width === this.width - 1;
    }

    locIsLeftCol(loc) {
        return loc % this.width === 0;
    }
}

export { BoardState as default }
