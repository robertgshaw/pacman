const requestHeader = 'REQUEST len=';
const bodyHeader = ', body={';

// REQUEST len=xx, body={"request":request}
function formatRequest(requestType, requestValue) {
    const bodyString = JSON.stringify({requestType:requestValue})
    return requestHeader + bodyString.length + bodyHeader + bodyString;   
}

function formatRequestMove(direction) {
    return formatRequest("move", direction);
}

function formatRequestQuit() {
    return formatRequest("quit", 1);
}

function handleEvent(board, event) {
    
    // if valid move event, return new state updated for the move
    if ('move' in event && 'pid' in event['move'] && 'loc' in  event['move'] && 'dir' in event['move']) {
        return handleEventMove(board, event['move']['pid'], event['move']['loc'], event['move']['dir']);
    } 
    
    // if valid add event, return new state  updated for the add
    else if ('add' in event && 'pid' in event['add'] && 'loc' in event['add']) {
        return handleEventAdd(board, event['add']['pid'], event['add']['loc']);
    } 
    
    // if valid quit event, return new state updated for the quit
    else if ('quit' in event && 'pid' in event['quit'] && 'loc' in event['quit']) {
        return handleEventQuit(board, event['quit']['pid'], event['quit']['loc']);
    } 

    console.log("invalid event, not updating");
    return squares;
}

function handleEventMove(board, playerId, location, direction) {
    board.movePlayer(playerId, location, direction);
}

function handleEventAdd(board, playerId, location) {
    board.addPlayer(playerId, location);
}

function handleEventQuit(board, playerId, location) {
    
}

export { handleEvent }