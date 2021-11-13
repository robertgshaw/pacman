import React from "react";
import './BoardView.css';
import { EMPTY, nodeTypeDictInv } from "./client-board";

export const BoardView = (props) => {
    
    const getPlayerType = (pacmanId, playerId) => {
        if (playerId === -1) {
            return "empty";
        } else if (playerId === pacmanId) {
            return "pacman";
        } else {
            return "ghost";
        }
    }

    const getSquareType = (nodeType, playerId) => {
        if (playerId !== EMPTY) {
            return "occupied";
        }
        return nodeTypeDictInv[nodeType];
    }

    const renderSquare = (i, j, width) => {
        const index = i * width + j;
        const playerId = props.squares[index].playerId;
        return (
            <SquareView
                key={"Square-"+index}
                playerType={getPlayerType(playerId, props.pacmanId)}
                squareType={getSquareType(props.squares[index].nodeType, playerId)}
                activeType={playerId === props.activePlayer ? "active" : "inactive"}
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
    let squareInnerView;
    if (props.squareType === "occupied"){
        squareInnerView = <SquareInnerViewOccupied playerType={props.playerType} activeType={props.activeType}/>
    } else {
        squareInnerView = <SquareInnerViewEmpty nodeType={props.squareType}/>
    }

    return <div className="square">{squareInnerView}</div>; 
};

const SquareInnerViewOccupied = (props) => {
    if (props.playerType === "pacman") {
        return <PacmanInnerView activeType={props.activeType}/>;
    } else {
        return <GhostInnerView activeType={props.activeType}/>;
    }
}

const SquareInnerViewEmpty = (props) => {
    if (props.nodeType === "blocked") {
        return <BlockedInnerView/>;
    } else if (props.nodeType === "coin") {
        return <CoinInnerView/>;
    } else {
        return <OpenInnerView/>;
    }
}

const PacmanInnerView = (props) => {
    return <div className={"inner pacman-inner " + (props.activeType === "active" ? "active-inner" : "")}></div>;
}

const GhostInnerView = (props) => {
    return <div className={"inner ghost-inner " + (props.activeType === "active" ? "active-inner" : "")}></div>;
}

const BlockedInnerView = () => {
    return <div className={"inner blocked-inner"}></div>;
}

const CoinInnerView = () => {
    return <div className={"inner coin-inner"}></div>;
}

const OpenInnerView = () => {
    return <div className={"inner open-inner"}></div>;
}

