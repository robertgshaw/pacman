// constants for the board below
const UP = 0;
const RIGHT = 1;
const DOWN = 2;
const LEFT = 3;
const EMPTY = -1;

const nodeTypeDict = { "coin" : 0, "open" : 1, "blocked" : 2 };
const nodeTypeDictInv = { 0 : "coin", 1 : "open", 2 : "blocked" };


// Board class
//      model component of the MVC framework
//      this class contains the data structure that holds the state + state update logic

class Board {

    // constructor(boardJSON)
    //      initializes a BoardState object from the json description of the board
    //      invariant that boardJSON is well formatted
    //      call 'validate JSON' before constructor to check things work
    constructor(boardJSON, activePlayer) {  
        this.activePlayer = activePlayer;
        this.pacmanId = boardJSON.pacman_id;
        this.pacmanScore = boardJSON.pacman_score;
        this.nPlayers = boardJSON.n_players;
        this.width = boardJSON.width;
    
        this.nodes = boardJSON.nodes.map((node) => {
            let obj = {};
            obj.playerId = node.player_id;
            obj.nodeType = node.node_type;
            return obj;
        });

        this.playerLocations = Array(this.nPlayers).fill(null);
        for (let i = 0; i < this.nodes.length; i++) {
            if (this.nodes[i].playerId !== EMPTY) {
                this.playerLocations[this.nodes[i].playerId] = i;
            }
        }
    }

    // setPacman(playerId)
    //      sets playerId as the pacmans
    setPacman(playerId) {
        this.pacmanId = playerId;
        return this;
    }

    // updateState(loc, playerId, newPlayer)
    //      utilitiy function that updates a node, handling updating each of the state vars
    //      (playerId, nodeType, and adding newPlayers)
    //      updates the node at "loc" to hold playerID + updates the nodeType (incrementing pacman score)
    //      updates the playerLocations array to hold the loc of the new player id

    updateState(loc, playerId, newPlayer=false) {
        // update nodes[loc]
        this.nodes[loc].playerId = playerId;

        // update playerLocations[playerId]
        if (playerId !== EMPTY) {

            // if new player, append to the playerLocations array
            if (newPlayer) {
                if (this.playerLocations.length !== playerId) {
                    console.log("ERROR: could not update node due to data inconsistency");
                } else {
                    this.playerLocations.push(loc);
                }

            // otherwise, just update the player location array
            } else {
                this.playerLocations[playerId] = loc;

                // if we had the pacman making the move, update the nodeTypes from coin to open + increment score
                if (playerId === this.pacmanId && this.nodes[loc].nodeType === nodeTypeDict["coin"]) {
                    this.nodes[loc].nodeType = nodeTypeDict["open"];
                    this.pacmanScore++;
                }
            }
        }
    }

    // removePlayer(playerId, loc)
    //      removes player [id] at location [loc] from the game, checking to make sure loc was correct
    removePlayer(playerId, loc) {
        if (this.playerLocations.length !== this.nPlayers) {
            console.log("ERROR: playerLocations length out of sync with nPlayers count");
            return null;
        } else if (playerId > this.playerLocations.length) {
            console.log("ERROR: invalid player ID");
            return null;
        } else if (this.playerLocations[playerId] === EMPTY) {
            return this;
        } else if (this.playerLocations[playerId] !== loc) {
            console.log("ERROR: loc is not equal to playerLocation[playerId]");
            return null;
        } else if(this.nodes[loc].playerId !== playerId) {
            console.log("this.nodes[loc].playerId != playerId");
            return null;
        } else {
            this.updateState(this.playerLocations[playerId], EMPTY);
            this.playerLocations[playerId] = EMPTY;
        }

        return this;
    }

    // addPlayer(playerId, loc)
    //      adds player [id] to location [loc]
    addPlayer(playerId, loc) {
        if (!this.locIsInBoard(loc)) {
            console.log("ERROR: cannot add player to this location since it is not in the board");
            return null;
        } else if (!this.locIsEmpty(loc)) {
            console.log("ERROR: cannot add player to this location since it is not empty");
            return null;
        } else {
            this.nPlayers++;
            this.updateState(loc, playerId, true);
        } 

        return this;
    }

    // movePlayer(playerId, dir)
    //      moves player [id] from location [loc] in direction [UP, DOWN, LEFT, RIGHT]
    movePlayer(playerId, dir) {
        let loc = this.playerLocations[playerId];
        switch(dir) {
            case UP:
                this.movePlayerHelper(playerId, loc, loc - this.width, (loc_) => this.locIsTopRow(loc_));
                break;
            case RIGHT:
                this.movePlayerHelper(playerId, loc, loc + 1, (loc_) => this.locIsRightCol(loc_));
                break;
            case LEFT:
                this.movePlayerHelper(playerId, loc, loc - 1, (loc_) => this.locIsLeftCol(loc_));
                break;
            case DOWN:
                this.movePlayerHelper(playerId, loc, loc + this.width, (loc_) => this.locIsBottomRow(loc_));
                break;
            default:
                console.log("ERROR: direction is not UP, DOWN, LEFT, or RIGHT");
                return null;
        }

        return this;
    }

    // movePlayerHelper (playerId, loc, newLoc, checkLocIsSide)
    //      helper function for movePlayer
    movePlayerHelper(playerId, loc, newLoc, checkLocIsSide) {
        if (!this.playerIsAtLoc(playerId, loc)) {
            console.log("ERROR: cannot move player since not at starting location");
        } else if (checkLocIsSide(loc)) {
            console.log("ERROR: cannot move player since new location is off the board");
        } else if (!this.locIsEmpty(newLoc)) {
            console.log("ERROR: cannot move player new location is not empty");
        } else {
            this.updateState(newLoc, playerId, false);
            this.updateState(loc, EMPTY, false);
        }
    }    

    playerIsAtLoc(id, loc) {
        return this.nodes[loc].playerId === id;
    }

    locIsEmpty(loc) {
        return this.nodes[loc].playerId === EMPTY;
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

export { Board, nodeTypeDictInv, UP, DOWN, LEFT, RIGHT, EMPTY }