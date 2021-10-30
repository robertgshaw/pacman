import React, {useState} from 'react';
import './App.css';
import Game from "./Game.js"

function App() {

  const [active, setActive] = useState(true);

  const shutdownGame = () => {
    setActive(false);
  };

  return (
    <div className="App">
      {active ? <Game /> : <span></span>}
      <button onClick={shutdownGame}>End Game</button>
    </div>
  );
}

export default App;
