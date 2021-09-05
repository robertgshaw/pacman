server_objects = server.o game.o board.o changelog.o event.o server_api.o exitpipe.o
client_objects = client.o client_api.o controller.o view.o
shared_objects = board.o utilities.o

# commands
all: server client

server: $(server_objects)
	g++ -o server.out $(server_objects) -pthread -std=c++14

client: $(client_objects) $(shared_objects)
	g++ -o client.out $(client_objects) $(shared_objects) -pthread -lncurses -std=c++14

.PHONY : clean
clean:
	rm server.out client.out $(server_objects) $(client_objects) $(shared_objects)

# server object file recipes
server.o: server/server.cc server/server_api.hh
	g++ -c server/server.cc -pthread -std=c++14

game.o: server/game.cc server/changelog.hh server/event.hh shared/board.hh
	g++ -c server/game.cc -pthread -std=c++14

changelog.o: server/changelog.cc server/event.hh
	g++ -c server/changelog.cc -pthread -std=c++14

event.o: server/event.cc
	g++ -c server/event.cc -pthread -std=c++14

server_api.o: server/server_api.cc server/game.hh server/exitpipe.hh
	g++ -c server/server_api.cc -pthread -std=c++14

exitpipe.o: server/exitpipe.cc
	g++ -c server/exitpipe.cc -pthread -std=c++14

# client object file recipes
client.o: client/client.cc client/client_api.hh client/controller.hh shared/utilities.hh 
	g++ -c client/client.cc -pthread -lncurses -std=c++14

client_api.o: client/client_api.cc client/controller.hh shared/utilities.hh  
	g++ -c client/client_api.cc -pthread -lncurses -std=c++14

controller.o: client/controller.cc client/view.hh shared/board.hh
	g++ -c client/controller.cc -pthread -lncurses -std=c++14

view.o: client/view.cc shared/board.hh
	g++ -c client/view.cc -pthread -lncurses -std=c++14

# shared object file recipes
board.o: shared/board.cc
	g++ -c shared/board.cc -pthread -lncurses -std=c++14

utilities.o: shared/utilities.cc
	g++ -c shared/utilities.cc -pthread -lncurses -std=c++14
