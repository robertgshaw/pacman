server: server.o game.o board.o changelog.o event.o helpers.o
	gcc -o server.out server.o game.o board.o changelog.o event.o helpers.o -pthread

server.o: server.cc
	gcc -cc server.cc -pthread

game.o: game.cc
	g++ -cc game.cc -pthread

board.o: board.cc
	g++ -cc board.cc -pthread

changelog.o: changelog.cc
	g++ -cc changelog.cc -pthread

event.o: event.cc
	g++ -cc event.cc -pthread

helpers.o: helpers.cc
	g++ -cc helpers.cc -pthread