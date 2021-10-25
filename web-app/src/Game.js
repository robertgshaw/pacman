import React, { useState, useEffect, useRef } from "react";
import BoardState from "./client-board.js";
import handleWebSocketEvent from "./client-api.js"
import './Game.css';

const Square = (props) => {
    return (
        <button className="square">
            {props.value}
        </button>
    ); 
};

const Board = (props) => {
    const renderSquare = (i, j, boardWidth) => {
        const index = i * boardWidth + j;
        return (
            <Square
                key={"Square-"+index}
                value={props.squares[index] === null ? "" : props.squares[index]}
            />
        );
    };

    let rows = [];
    for (let i = 0; i < props.boardWidth; i++) {
        let row = [];
        for (let j = 0; j < props.boardWidth; j++) {
            row.push(renderSquare(i, j, props.boardWidth));
        }
        rows.push(<div key={"boardRow-" + i} className="board-row">{row}</div>)
    }
    return rows;
};

const Game = () => {

    let board_ = new BoardState(5);
    const [squares, setSquares] = useState(board_.squares);

    const ws = useRef(null);

    useEffect(() => { 
        ws.current = new WebSocket("ws://localhost:27016");
        ws.current.binaryType = "arraybuffer";
        
        ws.current.onopen   = () => console.log("ws opened");
        ws.current.onclose  = () => console.log("ws closed");
        
        const wsCurrent = ws.current;

        return () => {
            wsCurrent.close();
        };

    }, []);

    useEffect(() => {
        if (!ws.current) return;

        ws.current.onmessage = (ev) => handleWebSocketEvent(ev);
    });

    const handlePlayerInput = e => {
        const keyPressed = e.key;
        let playerCommand = '';
        console.log("Pressed: " + keyPressed);

        switch (keyPressed) {
            case 'a':
                playerCommand = 'left';
                break;
            case 'w':
                playerCommand = 'up';
                break;
            case 's':
                playerCommand = 'down';
                break;
            case 'd':
                playerCommand = 'right';
                break;
            default:
                return;
        }
        
        ws.current.send("{'message':'"+playerCommand+"'}");        
    };

    return (
        <div>
            <Board boardWidth={5} squares={squares}/>
            <input name="playerRequest" onKeyDown={e => handlePlayerInput(e)} />
        </div>
    );
};

export default Game;
