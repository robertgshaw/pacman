import React from "react";
import './BoardView.css';

export const BoardView = (props) => {
    const renderSquare = (i, j, width) => {
        const index = i * width + j;
        return (
            <SquareView
                key={"Square-"+index}
                value={props.squares[index].playerId === -1 ? "" : props.squares[index].playerId}
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

const SquareView = (props) => {
    return (
        <button className="square">
            {props.value}
        </button>
    ); 
};

