server_objects = server.o game.o board.o changelog.o event.o server_api.o 
client_objects = client.o client_api.o controller.o view.o
shared_objects = board.o utilities.o

# commands
add: server client

server: $(server_objects)
	g++ -o server.out $(server_objects) -pthread

client: $(client_objects) $(shared_objects)
	g++ -o client.out $(client_objects) $(shared_objects) -pthread -lncurses

.PHONY : clean
clean:
	rm server.out client.out $(server_objects) $(client_objects) $(shared_objects)

# server object file recipes
server.o: server/server.cc server/server_api.hh server/game.hh
	g++ -c server/server.cc -pthread

game.o: server/game.cc server/changelog.hh server/event.hh shared/board.hh
	g++ -c server/game.cc -pthread

changelog.o: server/changelog.cc server/event.hh
	g++ -c server/changelog.cc -pthread

event.o: server/event.cc
	g++ -c server/event.cc -pthread

server_api.o: server/server_api.cc server/game.hh
	g++ -c server/server_api.cc -pthread

# client object file recipes
client.o: client/client.cc client/client_api.hh client/controller.hh shared/utilities.hh 
	g++ -c client/client.cc -pthread -lncurses

client_api.o: client/client_api.cc client/controller.hh shared/utilities.hh  
	g++ -c client/client_api.cc -pthread -lncurses

controller.o: client/controller.cc client/view.hh shared/board.hh
	g++ -c client/controller.cc -pthread -lncurses

view.o: client/view.cc shared/board.hh
	g++ -c client/view.cc -pthread -lncurses

# shared object file recipes
board.o: shared/board.cc
	g++ -c shared/board.cc -pthread -lncurses

utilities.o: shared/utilities.cc
	g++ -c shared/utilities.cc -pthread -lncurses
