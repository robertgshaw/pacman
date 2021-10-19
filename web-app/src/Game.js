import React, { useState, useEffect } from "react";
import io from "socket.io-client";
import './Game.css';

let endPoint = "http://localhost:3000";
let socket = io.connect(`${endPoint}`);

const Square = (props) => {
    return (
        <button className="square">
            {props.value}
        </button>
    ); 
}

const Board = (props) => {
    const renderSquare = (i, j) => {
        return (
            <Square
                key={i + ", " + j}
                value={props.squares[i][j] === null ? "" : props.squares[i][j]}
            />
        );
    };

    return (
        <div>
            {props.squares.map((row, i) => (
                <div className="board-row">
                    {row.map((square, j) => (renderSquare(i, j)))}
                </div>
            ))}
        </div>
    );     
  }

const Game = () => {
    
    const board_width = 5;

    const [squares, setSquares] = useState(Array.from(Array(board_width), () => Array(board_width).fill(null)));
    const [message, setMessage] = useState("");

    useEffect(() => { 
        getMessages(); 
    });
    
    const getMessages = () => {
        socket.on("message", msg => {

            const int_msg = parseInt(msg);
            if (!isNaN(int_msg) && int_msg < board_width * board_width && int_msg >= 0) {
                const squares_copy = squares.slice();
                const i = Math.floor(int_msg / board_width);
                const j = int_msg % board_width;
                squares_copy[i][j] = "X";
                setSquares(squares_copy);
            }
        });
    };

    const onChange = e => {
        setMessage(e.target.value);
    };

    const onClick = () => {
        if (message !== "") {
            socket.emit("message", message);
            setMessage("");
        } else {
            alert("Please Add A Message");
        }
    };

    return (
        <div>
            <Board 
                squares={squares}
            />

            <input value={message} name="message" onChange={e => onChange(e)} />
            <button onClick={() => onClick()}>Send Message</button>
        </div>
    );
  }

export default Game;
