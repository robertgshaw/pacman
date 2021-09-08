# Pacman: multiplayer version of pacman
This is a toy project setup to learn about concurrent systems (`C++`), web application design (`React`), containers (`docker`), cloud deployment (`AWS`).

## Current status:
The project currently allows multiple users to simultaneously connect to the server and move around a "Board" (the server is stateful given it is a live game)
When a player makes a move, the state of the board is automatically updated on all of the clients connected to the game
The client side uses a command line interface using NCURSES as the UI (currently in process of making this a broswer based game)
The client moves around the board by clicking 'awsd' and 'q' to quit
A user at the server's command line can write 'exit' and the game is shutdown, cleaning up resources

## Running the server:
Uses docker to build application.
1. Run `docker image build -t server-p .` to build the server
2. Run `docker run -p 6169:6169 server-p` to start the server

type `exit` into the command prompt to end the game

## Running the client: 
The client is not dockerized, since I am moving this to the browser
 
1. Run `make client` (I am using ubuntu)
2. Run with `./client.out [IP addr] [Port]` --- the port you want is 6169. if running on localhost use 127.0.0.1 for the IP addr

Then, you should have an active client with ncurses.
You can open up multiple terminals and run in order to see multiple cleints connected to the game. 

This will ultimately be moved up to the browser. (have a branch focused on this now)

## Project goal:
1. Demonstrate competancy with C++ patterns and `std`
2. Demonstrate competancy with concurrent programming (`thread`, `mutex`, `condition_variable`)
3. Demonstrate competancy with `docker`
4. Demonstrate familiarity with basic systems / networking concepts 
5. Demonstrate competancy with AWS
6. Demonstrate competancy with basic web app design
7. Demonstrate competancy with object oriented programming and polymorphism



## Immediate Next Steps
1) Create landing page
2) Put server onto AWS + enable access over open internet (vs today via) - starting with EC2 to learn
3) Potentially, play around with ECS / EKS

## Longer Term Goals:
    1) Build out a browser based client - will use React for this
    2) Put onto AWS (did a proof of concept here with ECS + got it working); will ultimately move there
    3) Build actual pacman features into the game

## Project Design
### Server:
##### The server code is decoupled into a few modules:
1) `server.cc`        Entry code that listens for new connections
2) `server_api.cc`    Manages active client connections and implements the C/S communication protocol
3) `game.cc`          Server Kernel that hold the data structures + implements the synchronization / message passing
4) `changelog.cc`     Underlying data structure holding events that occur during the game
5) `exitpipe.cc`      Manages the exit pipes for each worker thread (2 per connection). Enables signal to shutdown if the sever commandprompt says "exit"
6) `shared/board.cc`  Underlying data structre holding the shared state of the game

##### Server Functionality:
Concurrently do 4 things things:

1) listens for client requests (across multiple simulatenous connections)
2) process client request to update the shared state
3) pushes the changes to all of the clients, who update the client side representaiton of the game
4) listens to the command line for server admin to type "exit" - in which case it sends a signal to exit all connections

Each client connection is managed by the pair of threads
We use an "event streaming" architectures to pass the updated state to the clients - where the clients are passed
updates to the board + maintain their own representations of the games on the client side

##### Implementation:
Each client connection is handled by a pair of threads using the Reader/Writer pattern with blocking sockets. There are 2 shared data structures used to accomplish these actions, the synchrnization of which is managed by the Game class.
    
Data Stuctures
`Board` - this object holds the shared state of the game (under the covers it is a graph)
`Changelog` - this object holds a queue of events that happens (move quit add) --- these changes are passed to the client as messages and are processed

Threads 
`Writer Thread` --> listens to the changelog for new events happening; as events occur, 
 sends to the socket (+ then client side updates state)
`Reader Thread` --> listens to the socket for client requests (e.g. "move", "quit", "delete"); as 
 requests come in, update the board and log events in changelog

Additionally, we have a server user which has access to the server command line, accepting commands from the user. So far, we have implemented "exit" as a command the server user can type to end the game. We use a series of pipes to signal the connection threads (2 per connection) to shutdown when exit occurs. The worker reader threads therefore block on select() rather than recv and we create an "exit" event which is processed by the changelog.

### Client:
##### The client code is decoupled into a few modules:
1) `client.cc`          Entry code that connects the client to the server
2) `client_api.cc`      Manages the active connection to the server and implements the C/S communication protocol
3) `controller.cc`      Client Kernel that holds the data structures and manages the view
4) `view.cc`            View which manages the NCURSES UI
5) `shared/board.cc`    Underlying data structure which holds the state of the game (model)

###### Client Functionality
Concurrently do three things:
    
1) listens for user input and send request to the server to update state
2) listen for events from the server and process them to update the client side board state 
3) update the user interface to match the model representation of the board

### Implementation:
Each client connection is managed by the pair of threads using the reader / write pattern
"Event driven" architectures to pass the updated state to the clients, client responsible for maintaining client side representation of the game

The client utilizes the MVC structure
    The controller manages the game, utilizing two threads (reader/writer pattern)
    The board is the model (underlying data structure) which holds the state of the game
    The view is the visual interpretation of the model + gets necessary data updates passed from the controller

The controller uses the reader/writer paradigm threads to manage the game:
    "Writer Thread" --> this thread listens to the user input + wraps the user input in the C/S protocol 
        and sends the events to the socket
    "Reader Thread" --> this thread listens to the socket for events; as events come in; the board is updated + the 
        controller passes changes to the board to the view 
