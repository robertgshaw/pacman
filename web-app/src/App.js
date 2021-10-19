import React, {useState, useEffect} from 'react';
import './App.css';
import Game from "./Game.js"

function App() {
  const [currentTime, setCurrentTime] = useState(0);

  useEffect(() => {
    fetch('/api/time').then(res => res.json()).then(data => {
      setCurrentTime(data.time);
    });
  }, []);

  return (
    <div className="App">
      <Game />
      <p>The current time is {currentTime}.</p>
    </div>
  );
}

export default App;
