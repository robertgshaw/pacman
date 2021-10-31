import React from "react";
import './BoardView.css';

export const BoardView = (props) => {
    
    const getSquareBackgroundColor = (squareType, playerId, activePlayer) => {
        switch(squareType) {
            case "blocked": 
                return "background-gray";
            default:
                if(playerId === activePlayer) {
                    return "background-yellow";
                } else {
                    return "background-white";
                }
        }
    };

    const renderSquare = (i, j, width) => {
        const index = i * width + j;
        const playerId = props.squares[index].playerId;
        return (
            <SquareView
                key={"Square-"+index}
                value={playerId === -1 ? "" : playerId}
                backgroundColor={getSquareBackgroundColor(props.squares[index].type, playerId, props.activePlayer)}
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
        <button className={"square " + props.backgroundColor}>
            {props.value}
        </button>
    ); 
};

