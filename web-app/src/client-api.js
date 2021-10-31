import { Board, UP, DOWN, LEFT, RIGHT } from "./client-board.js";

// client-api.js
//      this module contains the code for parsing messages from the server + passing to the handlers
//      this module contains the code for creating requests to send to the server

/*  *****************************************************************
    *****************************************************************
    ************************ FROM SERVER API ************************
    *****************************************************************
    ***************************************************************** */

// handleWebSocketEvent(board, ev)
//      handles the events from the raw json format, passing to the controller:
//          (A) move:	moves the player in the board
//			(B) add:	adds a new player to the board
//			(C) quit:	removes a player from the board
//			(D) exit: 	exits the game
//
//          parses the WebSocket event into the json message 
//          passes the serverCommand JSON data to the server command handler
//          returns an updated board object
export function handleWebSocketEvent(board, ev) {    
    if (ev.data instanceof ArrayBuffer) {
        try {
            return handleServerCommand(JSON.parse(arrayBufferToString(ev.data)), board);
        } catch (e) {
            console.log("ERROR: unable to parse JSON recieved from WebSocket.");
            console.log(e);
            return null;
        }
    } else {
        console.log("ERROR: messages from Socket is not of type ArrayBuffer.");
        return null;
    }
}

// handleServerCommand(serverCommand, board)
//      inteprets the command sent form the server and passes to the board state handler
//      ultimately calls the model's update functionality
//      returns the board in a new state
function handleServerCommand(serverCommand, board) {
    if (board === null && !('board' in serverCommand)) {
        console.log("ERROR: board has not been intialized");
        return null;

    } else if ('board' in serverCommand) {
        return handleInitCommand(serverCommand.board, serverCommand.pid, board);

    } else if ('move' in serverCommand) {
        return handleCommand(serverCommand.move, board, 'pid', 'dir', (board_, playerId, dir) => {
            const updatedBoard = board.movePlayer(playerId, dir);
            return updatedBoard;
        });

    } else if ('add' in serverCommand) {
        return handleCommand(serverCommand.add, board, 'pid', 'loc', (board_, playerId, loc) => {
            const updatedBoard = board.addPlayer(playerId, loc);
            return updatedBoard;
        });

    } else if ('quit' in serverCommand) {
        return handleCommand(serverCommand.quit, board, 'pid', 'loc', (board_, playerId, loc) => {
            const updatedBoard = board.removePlayer(playerId, loc);
            return updatedBoard;
        });

    } else if ('exit' in serverCommand) {
        return 'exit';

    } else {
        console.log("ERROR: unknown command recieved from the server");
        return null;
    }
}

function handleInitCommand(boardCommand, activePlayer, board) {
    if (board === null) {
        return new Board(boardCommand, activePlayer);

    } else {
        console.log("ERROR: board has already been initialized.");
        return null;
    }
}

function handleCommand(command, board, jsonKey1, jsonKey2, handler) {
    if (!(jsonKey1 in command) || !(jsonKey2 in command)) {
        console.log("ERROR: command has unexpected format");
        return null;

    } else {
        return handler(board, command[jsonKey1], command[jsonKey2]);
    }
}

/*  *****************************************************************
    *****************************************************************
    ************************ TO SERVER API ************************
    *****************************************************************
    ***************************************************************** */

// formatClientRequest(requestType, modifier)
//      formats the client request into the protocol API which is of form:
//      REQUEST len=xx, body={"requestType":request}

//      "move" --- formatClientRequest("move", ["right", "up", "left", "down"])
//      "quit" --- formatClientRequest("quit")
export function formatClientRequest(requestType, modifier=1) {        
    
    let requestValue = null;

    if (requestType === "quit") {
        requestValue = 1;
    } 
    
    else if (requestType === "move") {
        switch(modifier) {
            case "up":
                requestValue = UP;
                break;
            case "right":
                requestValue = RIGHT;
                break;
            case "down":
                requestValue = DOWN;
                break;
            case "left":
                requestValue = LEFT;
                break;
            default:
                console.log("ERROR: invalid move direction");
                return null;
        }
    } 
    
    else {
        console.log("ERROR: invalid client request type.");
        return null;
    }

    const requestHeader = 'REQUEST len=';
    const bodyHeader = ', body=';

    let bodyJSON = {};
    bodyJSON[requestType] = requestValue;
    const bodyString = JSON.stringify(bodyJSON);
    return requestHeader + bodyString.length + bodyHeader + bodyString;   
}

// arrayBufferToString(buffer) 
//      helper function translates binary data to string
function arrayBufferToString(buffer) {
    return String.fromCharCode.apply(null, new Uint8Array(buffer));
}