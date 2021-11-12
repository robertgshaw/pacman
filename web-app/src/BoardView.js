import React from "react";
import './BoardView.css';

const backgroundDict = {
    "blocked" : "blocked-square",
    "coin" : "coin-square",
    "open" : "open-square",
};

const colorDict = {
    "ghost" : "ghost-sqaure",
    "pacman" : "pacman-square",
    "empty" : "empty-square"
};

const activeDict = {
    "active" : "active-square",
    "inactive" : "inactive-square" 
};

export const BoardView = (props) => {
    
    const getSquareFormat = (type, pacmanId, playerId, activePlayer) => {
        const backgroundClass = backgroundDict[type];

        let colorClass = "";
        if (playerId === -1) {
            colorClass = colorDict["empty"];
        } else if (playerId === pacmanId) {
            colorClass = colorDict["pacman"];
        } else {
            colorClass = colorDict["ghost"];
        }

        const activeClass = playerId === activePlayer ? activeDict["active"] : activeDict["inactive"];
        return backgroundClass + " " + colorClass + " " + activeClass;
    };

    const renderSquare = (i, j, width) => {
        const index = i * width + j;
        const playerId = props.squares[index].playerId;
        return (
            <SquareView
                key={"Square-"+index}
                value={playerId === -1 ? "" : playerId}
                format={getSquareFormat(props.squares[index].type, props.pacmanId, playerId, props.activePlayer)}
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
        <button className={"square " + props.format}>
            {props.value}
        </button>
    ); 
};

