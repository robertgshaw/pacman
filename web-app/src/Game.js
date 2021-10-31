import React, { useState, useEffect, useRef } from "react";
import { handleWebSocketEvent, formatClientRequest } from "./client-api.js";
import { BoardView } from "./BoardView.js";

// Game Component
//      this component is the "Controller" of the MVC framework
//      does three main items:
//          (1) listens to WebSocket for Events from the server, passing to the API handler (w/ the model)
//          (2) listens to the KeyBoard for User Moves, passing to the API handler (w/ the model)
//          (3) holds the model (Board object) + view state (BoardView), triggering re-renderings on change

const Game = () => {

    const [squares, setSquares] = useState([]);

    const ws = useRef(null);
    const board = useRef(null);
    const isActive = useRef(false);

    // setup WebSocket, only call with ComponentWillMount
    useEffect(() => { 
        ws.current = new WebSocket("ws://localhost:27016");
        ws.current.binaryType = "arraybuffer";
        
        ws.current.onopen   = () => console.log("WebSocket opened");
        ws.current.onclose  = () => console.log("WebSocket closed");
        
        const wsCurrent = ws.current;

        // send quit message to server if still active + close the socket
        return function cleanup() {
            // note: we can safely call close more than once, since if socket already closed, it does nothing
            //      per https://developer.mozilla.org/en-US/docs/Web/API/WebSocket/close 
            quitGame();
            wsCurrent.close(); // precaution in case ws.current is chaged
        };

    }, []);

    // handles messages from WebSocket, passing to API handlers
    useEffect(() => {
        if (!ws.current) return;
        ws.current.onmessage = (ev) => {

            // pass event to API handler, updating the board
            board.current = handleWebSocketEvent(board.current, ev);
            
            // if we got an exit message from server, turn off
            if (board.current === 'exit') {
                setSquares([]);
                quitGame();

            // if we got an error parsing the server message, quit + turn off
            } else if (board.current === null) {
                setSquares([]);
                quitGame();

            // otherwise, update the view state and re-render
            } else {
                isActive.current = true;
                setSquares(board.current.nodes.slice());
            }
        }
    });

    // handles UserCommand, passing to the API handlers
    const handleKeyBoardEvent = (ev) => {
        let playerCommand = '';
        let isQuit = false;
        const commandDict = {'a':'left', 'w':'up', 's':'down', 'd':'right'};
        switch (ev.key) {
            case 'a':
            case 'w':
            case "s":
            case "d":
                playerCommand = commandDict[ev.key];
                break;
            case 'q':
                playerCommand = 'quit';
                isQuit = true;
                break;
            default:
                return;
        }
        
        // if user input was a quit, send quit message to the server
        if (isQuit) {
            setSquares([]);
            quitGame();

        // otherwise, send the move message to the server
        } else {
            ws.current.send(formatClientRequest("move", playerCommand));
        }
    };

    // helper function
    //      sends quit message to server if game is active
    //      cleans up resources (isActive, ws, board)
    const quitGame = () => {
        if (isActive.current) {
            ws.current.send(formatClientRequest("quit"));
            isActive.current = false; 
        }
        ws.current.close();
        board.current = null;
    }

    const displayBoard = () => {
        if (board.current === null) {
            return <BoardView boardWidth={0} squares={squares} activePlayer={0}/>
        } else {
            return <BoardView boardWidth={board.current.width} squares={squares} activePlayer={board.current.activePlayer}/>
        }
    }

    return (
        <div>
            {displayBoard()}
            <input name="userKeyBoardInput" onKeyDown={e => handleKeyBoardEvent(e)} />
        </div>
    );
};

export default Game;
