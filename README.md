# Pacman: multiplayer version of pacman
Project developed in Linux (Ubuntu distribution). Not tested for MacOS / Windows.

## Project goal:
    Demonstrate competancy with C++ patterns and std
    Demonstrate competancy with concurrent programming (threads, mutexes, cvs)
    Demonstrate familiarity with basic systems / networking concepts 
    Demonstrate competancy with object oriented programming and polymorphism
        NOTE: C++ is terrible for polymorphism

## Current status:
    The project currently allows multiple users to simultaneously connect to the server and move around a "Board". 
    When a player makes a move, the state of the board is automatically updated on all of the clients connected to the game. 
    The client side uses a command line interface using NCURSES as the UI. 
    The player moves around the board by clicking "awsd" and "q" to quit. 
    The server currently runs on the localhost.

## Longer Term Goals:
    1) Build pacman features into the game
    2) Allow server to hold multiple simultaneous games
    3) Put Server onto AWS and actually run over the internet (vs localhost) 
    4) Move the Client Side code into a browser

## Project Design
### Server:
##### The server code is decoupled into a few modules:
    1) server.cc        Entry code that listens for new connections
    2) server_api.cc    Manages active client connections and implements the C/S communication protocol
    3) game.cc          Server Kernel that hold the data structures + implements the synchronization / message passing
    4) changelog.cc     Underlying data structure holding events that occur during the game
    5) shared/board.cc  Underlying data structre holding the shared state of the game

##### Server Functionality:
    Concurrently do three things:
    
    1) listens for client requests (across multiple simulatenous connections)
    2) process client request to update the shared state
    3) pushes the changes to all of the clients, who update the client side representaiton of the game
    
    Each client connection is managed by the pair of threads
    We use an "event streaming" architectures to pass the updated state to the clients - where the clients are passed
        updates to the board + maintain their own representations of the games on the client side

##### Implementation:
Each client connection is handled by a pair of threads using the Reader/Writer pattern with the socket. There are 2 shared data structures used to accomplish these actions, the synchrnization of which is managed by the Game class.
    
    Data Stuctures
    Board - this object holds the shared state of the game (under the covers it is a graph)
    Changelog - this object holds a queue of events that happens (move quit add) --- these changes are passed to the client as messages and are processed

    Threads 
    "Writer Thread" --> listens to the changelog for new events happening; as events occur, 
        sends to the socket (+ then client side updates state)
    "Reader Thread" --> listens to the socket for client requests (e.g. "move", "quit", "delete"); as 
        requests come in, update the board and log events in changelog

### Client:
##### The client code is decoupled into a few modules:
    1) client.cc        Entry code that connects the client to the server
    2) client_api.cc    Manages the active connection to the server and implements the C/S communication protocol
    3) controller.cc    Client Kernel that holds the data structures and manages the view
    4) view.cc          View which manages the NCURSES UI
    5) shared/board.cc  Underlying data structure which holds the state of the game (model)

###### Client Functionality:
    Concurrently do four things:
    
    1) listens for user input and send request to the server to update state
    2) listen for events from the server and process them to update the client side board state 
    3) update the user interface to match the model representation of the board

### Implementation:
    Each client connection is managed by the pair of threads using the reader / write pattern
    "Event streaming" architectures to pass the updated state to the clients, client responsible for maintaining client side representation of the game

    The client utilizes the MVC structure
        The controller manages the game, utilizing two threads (reader/writer pattern)
        The board is the model (underlying data structure) which holds the state of the game
        The view is the visual interpretation of the model + gets necessary data updates passed from the controller

    The controller uses the reader/writer paradigm threads to manage the game:
        "Writer Thread" --> this thread listens to the user input + wraps the user input in the C/S protocol 
            and sends the events to the socket
        "Reader Thread" --> this thread listens to the socket for events; as events come in; the board is updated + the 
            controller passes changes to the board to the view 
            
## Running Instructions:
Navigate to the pacman directory in a terminal
1. Run `make server` to compile the server 
2. Run `make client` to compile the client
3. Run `./server.out` to launch the server (opens a socket on the localhost)

Open a new terminal
1. Run `./client.out` --> use awsd to move around the board; q to quit

Open another terminal
1. Run `./client.out` --> use awsd to move around the board; q to quit

You can connect up to 5 terminals at once 
You will see that both client terminals are connected to the same server and are updated

## Docker
I developed this project on a Windows laptop, using some infrastructure that CS61 at Harvard setup using Docker to launch a Linux OS inside a container. Utimately may plan to move project to an actual containerized enviornment, but the command I use on my laptop is below:

`docker run --network="host" -it --rm -v C:\Users\rober\cs61-f20-psets-robertgshaw\pacman:/home/cs61-user/pacman cs61:latest`
