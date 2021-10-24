import React, { useState, useEffect, useRef } from "react";
import BoardState from "./client-board.js";
// import handleEvent from "client-api.js"
import './Game.css';

const Square = (props) => {
    return (
        <button className="square">
            {props.value}
        </button>
    ); 
};

const Board = (props) => {
    const renderSquare = (i) => {
        return (
            <Square
                key={i}
                value={props.squares[i] === null ? "" : props.squares[i]}
            />
        );
    };

    return (
        <div>
            {props.squares.map((sq,i) => renderSquare(i))}
        </div>
    );     
};

const Game = () => {

    let board_ = new BoardState(5);
    const [squares, setSquares] = useState(board_.squares);

    const ws = useRef(null);

    useEffect(() => { 
        ws.current = new WebSocket("ws://localhost:27016")
        
        ws.current.onopen   = () => console.log("ws opened");
        ws.current.onclose  = () => console.log("ws closed");
        
        const wsCurrent = ws.current;

        return () => {
            wsCurrent.close();
        };

    }, []);

    useEffect(() => {
        if (!ws.current) return;

        ws.current.onmessage = msg => {
            const incomingMessage = `Message from WebSocket: ${msg.data}`;
            console.log(incomingMessage);
        }
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
        
        ws.current.send("message", playerCommand);        
    };

    return (
        <div>
            <Board squares={squares}/>
            <input name="playerRequest" onKeyDown={e => handlePlayerInput(e)} />
        </div>
    );
};

export default Game;
